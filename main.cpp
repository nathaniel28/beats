// under GPL-2.0

#include <bit>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <vlcpp/vlc.hpp>

#include "keys.hpp"
#include "chart.hpp"
#include "util.hpp"

#include "argparse/argparse.h"
#include "shaders/sources.h"

#ifdef _WIN64
#include <windows.h>
#endif

void usage() {
	std::cout << "Usage: ./beats [OPTION]... <CHART>\n"
		"Play a .chart file. Options are in milliseconds unless otherwise specified.\n"
		"  -t, --view-timespan [=650]      How far into the future you should be able to\n"
		"                                    see. Smaller view timespan equals faster\n"
		"                                    falling notes. 650 ms is about 4x speed in\n"
		"                                    Rhythm Plus.\n"
		"  -a, --audio-offset [=0]         Makes notes fall earlier (if positive) or\n"
		"                                    later (if negative). Use it to calibrate\n"
		"                                    delay for bluetooth headphones.\n"
		"  -v, --video-offset [=0]         Makes you have to hit the note earlier (if\n"
		"                                    positive) or later (if negative).\n"
		"  -s, --strike-timespan [=250]    Smaller strike timespan equals easier game.\n"
		"  -d, --min-initial-delay [=800]  If a song has notes before this timestamp,\n"
		"                                    delay playback.\n"
		"  -x [=100]                       Length of each column (total screen length is\n"
		"                                    4 times this if you have 4 columns).\n"
		"  -y [=600]                       Height of each column.\n"
		"      --note-height [=8]          Height of each non hold note\n"
		"      --no-vsync                  Disable vsync.\n";
	// in the future, add -m, --music
}

// behold the magnificent 400 line main function
int main(int argc, char **argv) {
	// parsing command line arguments...
	if (argc <= 1) {
		usage();
		return 0;
	}
	// if you update this, update the README and usage() too
	long display_timespan_opt = 650;
	long audio_offset_opt = 0;
	long video_offset_opt = 0;
	long strike_timespan_opt = 250;
	long min_initial_delay_opt = 800;
	long x_opt = 100;
	long y_opt = 600;
	long note_height_opt = 8;
	unsigned char no_vsync_opt = 0;
	Option opts[] = {
		OPT('t', "view-timespan", OPT_LONG, &display_timespan_opt),
		OPT('a', "audio-offset", OPT_LONG, &audio_offset_opt),
		OPT('v', "video-offset", OPT_LONG, &video_offset_opt),
		OPT('s', "strike-timespan", OPT_LONG, &strike_timespan_opt),
		OPT('d', "min-initial-delay", OPT_LONG, &min_initial_delay_opt),
		OPT('x', nullptr, OPT_LONG, &x_opt),
		OPT('y', nullptr, OPT_LONG, &y_opt),
		OPT(0, "note-height", OPT_LONG, &note_height_opt),
		OPT(0, "no-vsync", OPT_BOOL, &no_vsync_opt),
	};
	int extra_args = argparse(argc, argv, opts, sizeof(opts) / sizeof(*opts));
	bool bad_options = false;
	if (display_timespan_opt < 50) {
		std::cout << "View timespan is too short!\n";
		bad_options = true;
	}
	if (strike_timespan_opt < 10) {
		std::cout << "Strike timespan is too short!\n";
		bad_options = true;
	}
	if (x_opt < 10) {
		std::cout << "Columns are too thin!\n";
		bad_options = true;
	}
	if (y_opt < 100) {
		std::cout << "Columns are too short!\n";
		bad_options = true;
	}
	if (note_height_opt < 4) {
		std::cout << "Notes are too small!\n";
		bad_options = true;
	}
	if (extra_args < 2) {
		std::cout << "You didn't specify a chart file!\n";
		bad_options = true;
	}
	if (bad_options) {
		usage();
		return -1;
	}
	char *chart_file = argv[1]; // argparse moves around argv
	// the following variables are times in milliseconds
	uint64_t min_initial_delay = min_initial_delay_opt; // the least amount of time before you need to hit the first note
	uint64_t strike_timespan = strike_timespan_opt; // pressing a key will result in a strike if the next note in the key's column is more than strike_timespan ms in the future or past
	uint64_t display_timespan = display_timespan_opt; // notes at the top of the screen will be display_timespan ms in the future
	uint64_t min_delay_per_frame = 5; // wait at least this long between each frame render (if vsync is on, this will not change things unless you want to draw slower than the refresh rate)
	int64_t audio_offset = audio_offset_opt; // given the delay of the headphones/speakers and the player's audio reaction time
	int64_t video_offset = video_offset_opt; // given the delay of the keyboard and the player's visual reaction time
	// audio_offset and video_offset are optimal if the mean of all deltas
	// returned by close_note is 0.

	std::fstream in;
	in.open(chart_file, std::ios_base::in | std::ios_base::binary);
	if (!in.is_open()) {
		std::cout << "failed to open chart file\n";
		return -1;
	}
	Chart ch(y_opt, x_opt, note_height_opt);
	if (ch.deserialize(in)) {
		std::cout << "failed to deserialize file\n";
		return -1;
	}
	in.close();
	std::span<const int> column_bindings = bindings_of(ch.total_columns());
	if (column_bindings.size() == 0) {
		std::cout << "invalid number of columns\n";
		return -1;
	}

	// set up SDL with the ability to use OpenGL
	int err = SDL_Init(SDL_INIT_VIDEO);
	if (err != 0) {
		std::cout << SDL_GetError() << '\n';
		return -1;
	}
	defer { SDL_Quit(); };
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_Window *win = SDL_CreateWindow("beats", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ch.width(), ch.height(), SDL_WINDOW_OPENGL);
	if (!win) {
		std::cout << SDL_GetError() << '\n';
		return -1;
	}
	defer { SDL_DestroyWindow(win); };
	SDL_GLContext gl_ctx = SDL_GL_CreateContext(win);
	if (!gl_ctx) {
		std::cout << SDL_GetError() << '\n';
		return -1;
	}
	defer { SDL_GL_DeleteContext(gl_ctx); };
	SDL_GL_SetSwapInterval(!no_vsync_opt);
	glewExperimental = GL_TRUE;
	GLenum gerr = glewInit(); // must be called after SDL_GL_CreateContext
	if (gerr != GLEW_OK) {
		std::cout << "failed to initialize GLEW: " << glewGetErrorString(gerr) << '\n';
		return -1;
	}

	char *c = chart_file;
	char *last_dot = c;
	while (*c) {
		if (*c == '.')
			last_dot = c;
		c++;
	}
	*last_dot = '\0';
	// setup libVLC
	// VLC either has a race condition or I'm not destroying something
	// properly because sometimes on exit there is a use after free
	// in some VLC clean up functions... Either way, it's not a priority
	// for me to fix because it only happens on program exit ¯\_(ツ)_/¯
	VLC::Instance vlci(0, nullptr);
	VLC::Media media(vlci, chart_file, VLC::Media::FromPath);
	// I've tried Media::state to find out if it couldn't open the file...
	// but it returns 0 if the file exists or not, so TODO figure out how
	// to handle errors
	VLC::MediaPlayer mp(media);

	// compile and link shaders (source code is stored as const char[],
	// #included from shaders/sources.h, created with ./process_shaders.py)
	// this shader converts pixel coordinates (as ints) to floats on
	// the range [-1.0, 1.0]
	GLuint note_prog = create_program(note_vert_src, note_frag_src);
	if (!note_prog)
		return -1;
	defer { glDeleteProgram(note_prog); };
	glUseProgram(note_prog);
	glUniform2f(0, static_cast<float>(ch.width()), static_cast<float>(ch.height())); // sets uniform "scrSize"
	glUniform3f(1, 0.486, 0.729, 0.815); // sets uniform "color"
	// The following program makes the columns light up when you press them
	GLuint postprocess_prog = create_program(highlight_vert_src, highlight_frag_src);
	if (!postprocess_prog)
		return -1;
	defer { glDeleteProgram(postprocess_prog); };
	glUseProgram(postprocess_prog);
	glUniform1f(0, static_cast<float>(ch.total_columns())); // sets uniform "nCols"

	glViewport(0, 0, ch.width(), ch.height());
	glClearColor(0.0, 0.0, 0.0, 1.0);

	GLuint vaos[2];
	glGenVertexArrays(sizeof(vaos) / sizeof(*vaos), vaos);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to generate vertex array objects\n";
		return -1;
	}
	defer { glDeleteVertexArrays(sizeof(vaos) / sizeof(*vaos), vaos); };
	const GLuint note_vao = vaos[0];
	const GLuint scr_quad_vao = vaos[1];
	GLuint buffers[3];
	glGenBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to generate buffers\n";
		return -1;
	}
	defer { glDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers); };
	const GLuint note_vbo = buffers[0];
	const GLuint note_ebo = buffers[1];
	const GLuint scr_quad_vbo = buffers[2];

	// we need a quad to fill the whole screen in order to draw a post-processed texture
	// the first two columns are normalized device coordinates for the quad
	// the second two columns are texture coordinates
	const float scr_quad[] = {
		-1.0,  1.0,  0.0, 1.0,
		-1.0, -1.0,  0.0, 0.0,
		 1.0, -1.0,  1.0, 0.0,

		-1.0,  1.0,  0.0, 1.0,
		 1.0, -1.0,  1.0, 0.0,
		 1.0,  1.0,  1.0, 1.0
	};
	glBindVertexArray(scr_quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, scr_quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(scr_quad), scr_quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));

	// now set up a framebuffer and a texture so we can render here instead
	// of directly to the screen this way, we can do post processing
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to create framebuffer\n";
		return -1;
	}
	defer { glDeleteFramebuffers(1, &fbo); };
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLuint cbuf_tex;
	glGenTextures(1, &cbuf_tex);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to generate texture\n";
		return -1;
	}
	defer { glDeleteTextures(1, &cbuf_tex); };
	glBindTexture(GL_TEXTURE_2D, cbuf_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ch.width(), ch.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cbuf_tex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "failed to initialize framebuffer\n";
		return -1;
	}
	// no need to unbind the framebuffer since we bind each time we render

	// that was a lot of OpenGL setup...

	// keybindings; ks.data stores an action associated with the keypress
	Keystates ks;
	// idk why these arrays aren't initialized by ks's default constructor but they definitely aren't
	ks.pressed.fill(false);
	ks.data.fill(ACT_NONE);
	for (unsigned i = 0; i < column_bindings.size(); i++) {
		ks.data[column_bindings[i]] = ACT_COLUMN(i);
	}
	ks.data[SDL_SCANCODE_SPACE] = ACT_PAUSE;

	int64_t initial_delay = 0;
	Note *first = ch.first_note();
	if (!first) {
		std::cout << "This chart does not have any notes! What?!\n";
		return -1;
	}
	uint64_t first_time = first->timestamp;
	if (first_time < min_initial_delay) {
		std::cout << "enforcing minimum delay\n";
		initial_delay = min_initial_delay - first_time;
	}

	uint64_t score = 0;
	uint64_t max_score = 0;

	SDL_RaiseWindow(win);

	// threads are joined during deconstruction,
	// so as must be part of this scope
	std::future<void> as;
	if (initial_delay) {
		audio_offset -= initial_delay;
		// VLC::MediaPlayer::play only calls libvlc_media_player_play, which
		// does not have documentation on its thread safety. So naturally, we
		// are being irresponsible and assuming it is thread safe. It likely
		// is, since libVLC is multithreaded under the hood.
		as = std::async(std::launch::async, [&]{
			std::this_thread::sleep_for(std::chrono::milliseconds(initial_delay));
			mp.play();
		});
	} else {
		mp.play();
	}

	bool chart_paused = false;
	uint64_t pause_frame;
	uint64_t start_chart = SDL_GetTicks64();
	// finally, begin event and render loop
	while (1) {
		uint64_t start_frame = SDL_GetTicks64();
		int64_t song_offset = start_frame - start_chart + audio_offset;

		bool update_accuracy = false;

		// oh boy, we're back to 7 levels of indentation!
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_KEYDOWN:
			{
				// I need a new scope here because C++ throws a fit if you
				// declare variables after a label without one
				bool is_held = ks.set(ev.key.keysym.scancode, true);
				int action = ks.data[ev.key.keysym.scancode];
				if (action == ACT_NONE)
					break;
				if (action > MAX_COLUMN) {
					// not a column press, so figure out what it is now
					switch (action) {
					case ACT_PAUSE:
						if (chart_paused) {
							start_chart += start_frame - pause_frame;
							song_offset = start_frame - start_chart + audio_offset;
							chart_paused = false;
							// possibly use setTime to re-sync the song
							// this seems unnecessary from my testing,
							// however.
							//mp.setTime();
							mp.play();
						} else {
							mp.pause();
							pause_frame = start_frame;
							chart_paused = true;
						}
						break;
					default:
						std::cerr << "unknown action " << action << '\n';
						return -1;
					}
					break;
				}
				// now we know that action is a column press
				// for a note; but don't check for gameplay
				// input when the game is paused!
				if (is_held)
					break;
				int column_index = COLUMN_ACT_INDEX(action);
				if (chart_paused || column_index >= ch.total_columns())
					break;
				auto [found, delta] = ch.close_note(column_index, song_offset + video_offset, strike_timespan);
				if (found) {
					uint64_t note_score = strike_timespan - (delta < 0 ? -delta : delta);
					found->active = false;
					score += note_score;
				} else {
					std::cout << "strike at " << song_offset << "!\n";
				}
				max_score += strike_timespan;
				update_accuracy = true;
				break;
			}
			case SDL_KEYUP:
				ks.set(ev.key.keysym.scancode, false);
				break;
			case SDL_QUIT:
				return 0;
			}
		}

		if (chart_paused) {
			// the framebuffer fbo's texture stores the last frame
			// drawn; we can draw it repeatedly for a still image;
			// then, we can draw the menu above it.
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(postprocess_prog);
			glBindVertexArray(scr_quad_vao);
			glBindTexture(GL_TEXTURE_2D, cbuf_tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		} else {
			bool update_last_press = false;
			for (int i = 0; i < ch.total_columns(); i++) {
				int state = ks.pressed[column_bindings[i]];
				if (state) {
					ch.last_press[i] = song_offset;
					update_last_press = true;
				} else if (ch.holding_columns[i]) {
					Note *note = ch.unhold(i);
					if (note) {
						int64_t delta = song_offset + video_offset - (note->timestamp + note->hold_duration);
						if (delta < 0)
							delta = -delta;
						if (static_cast<uint64_t>(delta) < strike_timespan)
							score += strike_timespan - delta;
						max_score += strike_timespan;
						update_accuracy = true;
					}
				}
				ch.holding_columns[i] = state;
			}

			// Releasing a hold note outside the strike_timespan does not
			// drop the note. idk it this is a feature or not.
			if (static_cast<uint64_t>(song_offset + video_offset) > strike_timespan) {
				int64_t d_time = song_offset + video_offset - strike_timespan;
				int dropped = ch.drop_before(d_time);
				if (dropped) {
					max_score += strike_timespan * dropped;
					std::cout << "missed " << dropped << " notes before " << d_time << '\n';
					update_accuracy = true;
				}
			}

			if (update_accuracy)
				std::cout << "accuracy: " << static_cast<double>(score) / static_cast<double>(max_score) * 100.0 << "% of " << strike_timespan << "ms\n";

			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(note_prog);
			ch.draw(song_offset, song_offset + display_timespan, note_vao, note_vbo, note_ebo);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glUseProgram(postprocess_prog);
			glUniform1ui(1, static_cast<uint32_t>(song_offset)); // sets uniform "now"
			if (update_last_press)
				glUniform1uiv(2, ch.last_press.size(), ch.last_press.data()); // sets uniform "times"
			glBindVertexArray(scr_quad_vao);
			glBindTexture(GL_TEXTURE_2D, cbuf_tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		SDL_GL_SwapWindow(win);

		// if vsync is on, we don't really need to do this
		uint64_t elapsed = SDL_GetTicks64() - start_frame;
		if (elapsed < min_delay_per_frame)
			SDL_Delay(min_delay_per_frame - elapsed);
	}
	return 0;
}

#ifdef _WIN64
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	(void) hInstance;
	(void) hPrevInstance;
	(void) pCmdLine;
	(void) nCmdShow;
	int err = main(__argc, __argv);
	std::cout << "Press Enter to continue...";
	std::cin.get();
	return err;
}
#endif
