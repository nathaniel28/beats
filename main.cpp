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

#include "shaders/sources.h"

// see https://stackoverflow.com/questions/32432450
// thank you pmttavara!
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }
#define DEFER_(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__LINE__) = defer_dummy{} *[&]()

// a point given a pixel coordinate on screen
// we do rely on the fact that a point is 64 bits, and a vector<Point> is packed
// (though I can't imagine a vector<Point> with each point consuming more or
// less than 64 bits)
struct Point {
	int32_t x;
	int32_t y;
};

struct Note {
	uint32_t timestamp; // in milliseconds
	uint32_t hold_duration; // 0 for a press
	bool active; // whether or not to consider the note needing to be pressed
};

class Column {
public:
	std::vector<Note> notes;
	Note *holding_note;
private:
	int note_index, drop_index;
public:
	Column();

	void emit_verts(int64_t t0, int64_t t1, int column_height, int note_width, int note_height, int column_offset, std::vector<Point> &points, std::vector<uint32_t> &indices);

	std::pair<Note *, int64_t> close_note(uint64_t time, uint64_t threshhold);

	int drop_before(int64_t time);

	//void reset();
};

Column::Column() : notes(0) {
	holding_note = nullptr;
	note_index = 0;
	drop_index = 0;
}

void Column::emit_verts(int64_t t0, int64_t t1, int column_height, int note_width, int note_height, int column_offset, std::vector<Point> &points, std::vector<uint32_t> &indices) {
	int i = note_index;
	int max = notes.size();
	while (i < max && static_cast<int64_t>(notes[i].timestamp) < t1) {
		if (static_cast<int64_t>(notes[i].timestamp + notes[i].hold_duration) < t0) {
			// we can do this because notes are kept ordered by time
			note_index = i; // next time, don't bother with notes before this
		} else if (notes[i].active || notes[i].hold_duration > 0) { // we always draw hold notes
			const int32_t x0 = note_width * column_offset;
			const int32_t x1 = x0 + note_width;
			const int32_t y1 = ((column_height * (static_cast<int64_t>(notes[i].timestamp) - t0)) / (t1 - t0));
			const int32_t y0 = y1 + (notes[i].hold_duration ? notes[i].hold_duration : note_height);
			const uint32_t sz = points.size();
			indices.emplace(indices.end(), sz + 0);
			indices.emplace(indices.end(), sz + 1);
			indices.emplace(indices.end(), sz + 2);
			indices.emplace(indices.end(), sz + 0);
			indices.emplace(indices.end(), sz + 2);
			indices.emplace(indices.end(), sz + 3);
			points.emplace(points.end(), x0, y0);
			points.emplace(points.end(), x0, y1);
			points.emplace(points.end(), x1, y1);
			points.emplace(points.end(), x1, y0);
		}
		i++;
	}
}

std::pair<Note *, int64_t> Column::close_note(uint64_t time, uint64_t threshhold) {
	int max = notes.size();
	if (max == 0)
		return {nullptr, 0};
	uint64_t t_min = time > threshhold ? time - threshhold : 0;
	int i = note_index;
	Note *least = nullptr;
	while (i >= 0 && notes[i].timestamp >= t_min) {
		if (notes[i].active)
			least = &notes[i];
		i--;
	}
	if (least) {
		return {least, time - least->timestamp};
	}
	uint64_t t_max = time + threshhold;
	i = note_index;
	while (i < max && notes[i].timestamp <= t_max) {
		if (notes[i].timestamp >= t_min && notes[i].active) {
			return {&notes[i], time - notes[i].timestamp};
		}
		i++;
	}
	return {nullptr, 0};
}

int Column::drop_before(int64_t time) {
	int max = notes.size();
	int dropped = 0;
	while (drop_index < max && static_cast<int64_t>(notes[drop_index].timestamp) < time) {
		// it took restraint to avoid dropped += notes[drop_index].active
		if (notes[drop_index].active)
			dropped++;
		drop_index++;
	}
	return dropped;
}

#define ACT_NONE 0
#define ACT_COLUMN(n) (n + 1)
#define COLUMN_ACT_INDEX(n) (n - 1)
#define MAX_COLUMN 8
#define ACT_PAUSE (MAX_COLUMN + 1)

class Chart {
	std::vector<Column> columns;
	std::vector<Point> points; // points of rectangles to draw notes
	std::vector<uint32_t> indices; // indices for the EBO to help draw points
public:
	std::vector<uint32_t> last_press; // last press of a column, millisecond offset from song start, u32 for GLSL
	std::vector<uint8_t> holding_columns; // columns held last frame; uint8_t instead of bool to avoid weird feature
private:
	int column_height;
	int note_width;
	int note_height;

	// file signature
	static const uint64_t magic = 0xF1E0007472616863;
	static const uint32_t version = 2;

public:
	// calling the constructor does not make a Chart ready to use. (sorry)
	// you must call deserialize next
	Chart(uint32_t col_height, uint32_t note_width_, uint32_t note_height_);

	// on success this function returns 0 and this Chart is safe to use
	// calls istream::exceptions
	int deserialize(std::istream &);

	//int serialize(std::ostream &);

	// draw the notes from t0 to t1, also updates note_index
	// future calls should be made with a greater t0 and t1
	void draw(int64_t t0, int64_t t1, GLuint vao, GLuint vbo, GLuint ebo);

	// returns the closest *unpressed* note and the absolute time difference
	// between time and that note's timestamp, where column is the offset of
	// the column to look for notes in, and threshhold is how far in the
	// past/future to search
	std::pair<Note *, int64_t> close_note(int column, uint64_t time, uint64_t threshhold);

	Note *unhold(int column);

	Note *first_note();

	// similar to draw calls,
	// future calls should be made with a greater time
	int drop_before(int64_t time);

	int width();

	int height();

	int total_columns();
};

Chart::Chart(uint32_t col_height, uint32_t note_width_, uint32_t note_height_) : columns(0), points(512), indices(768), last_press(0), holding_columns(0) {
	column_height = col_height + note_height_;
	note_width = note_width_;
	note_height = note_height_;
}

#define READ(stream, dat, sz) stream.read(reinterpret_cast<char *>(dat), sz)

// expects a file with the following format:
// magic (8 bytes), version (4 bytes), column count (4 bytes)
// followed by column count number of
//   column index (4 bytes), note count (4 bytes)
//   followed by note count number of
//     timestamp (4 bytes), hold duration (4 bytes)
int Chart::deserialize(std::istream &is) {
	is.exceptions(std::ostream::failbit | std::ostream::badbit);
	try {
		// eventually, use endian.h to be extra standard, but for now,
		// assume little endian
		struct {
			uint64_t magic;
			uint32_t version;
			uint32_t column_count;
		} header; // better be packed right!
		READ(is, &header, sizeof(header));
		if (header.magic != Chart::magic) {
			std::cerr << "invalid magic " << header.magic << '\n';
			return -1;
		}
		if (header.version != Chart::version) {
			std::cerr << "invalid version " << header.version << '\n';
			return -1;
		}
		if (header.column_count >= MAX_COLUMN || header.column_count == 0) {
			std::cerr << header.column_count << " is a bad number of columns\n";
			return -1;
		}
		columns.resize(header.column_count);
		last_press.resize(header.column_count);
		holding_columns.resize(header.column_count);

		for (uint32_t i = 0; i < header.column_count; i++) {
			uint32_t column_index;
			READ(is, &column_index, sizeof(column_index));
			if (column_index >= header.column_count) {
				std::cerr << "column index " << column_index << " out of range " << header.column_count << '\n';
				return -1;
			}
			uint32_t note_count;
			READ(is, &note_count, sizeof(note_count));
			columns[column_index].notes.resize(note_count);

			uint32_t last_timestamp = 0;
			Note *n = &columns[column_index].notes[0];
			while (note_count--) {
				READ(is, &n->timestamp, sizeof(n->timestamp));
				if (n->timestamp < last_timestamp) {
					std::cerr << "unordered timestamps\n";
					return -1;
				}
				READ(is, &n->hold_duration, sizeof(n->hold_duration));
				n->active = true;
				last_timestamp = n->timestamp + n->hold_duration;
				n++;
			}
		}
	} catch (const std::ios_base::failure &err) {
		return err.code().value();
	}
	return 0;
}

void Chart::draw(int64_t t0, int64_t t1, GLuint vao, GLuint vbo, GLuint ebo) {
	points.clear(); // does not deallocate memory, though
	indices.clear(); // ditto
	int sz = columns.size();
	for (int i = 0; i < sz; i++) {
		columns[i].emit_verts(t0, t1, column_height, note_width, note_height, i, points, indices);
	}
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * points.size(), points.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), 0); // why does GL_FLOAT work but GL_INT doesn't, since I'm giving it ints
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

std::pair<Note *, int64_t> Chart::close_note(int column, uint64_t time, uint64_t threshhold) {
	std::pair<Note *, int64_t> res = columns[column].close_note(time, threshhold);
	Note *note = std::get<0>(res);
	if (note && note->hold_duration > 0)
		columns[column].holding_note = note;
	return res;
}

Note *Chart::unhold(int column) {
	Note *res = columns[column].holding_note;
	columns[column].holding_note = nullptr;
	return res;
}

Note *Chart::first_note() {
	Note *first = nullptr;
	for (Column &col : columns) {
		if (col.notes.size() == 0)
			continue;
		Note *n = &col.notes[0];
		if (!first || first->timestamp > n->timestamp)
			first = n;
	}
	return first;
}

int Chart::drop_before(int64_t time) {
	int dropped = 0;
	for (Column &col : columns) {
		dropped += col.drop_before(time);
	}
	return dropped;
}

int Chart::width() {
	return total_columns() * note_width;
}

int Chart::height() {
	return column_height - note_height;
}

int Chart::total_columns() {
	return columns.size();
}

struct Keystates {
	std::array<int, SDL_NUM_SCANCODES> data;
	std::array<bool, SDL_NUM_SCANCODES> pressed;

	Keystates() = default;

	// sets the scancode to change and returns the previous value (true if the key is held)
	bool set(int scancode, bool change);
};

bool Keystates::set(int scancode, bool change) {
	if (scancode >= SDL_NUM_SCANCODES)
		return false;
	bool res = pressed[scancode];
	pressed[scancode] = change;
	return res;
}

void log_shader_err(GLuint shader) {
	GLchar log[256];
	GLsizei log_len;
	glGetShaderInfoLog(shader, sizeof(log), &log_len, log);
	std::cout.write(log, log_len) << '\n';
	if (log_len == sizeof(log))
		std::cout << "\n(log truncated)\n";
}

GLuint create_shader(GLenum type, const GLchar *src, GLint len) {
	GLuint shader = glCreateShader(type);
	if (!shader)
		return 0;
	const char *const srcs[] = { src };
	glShaderSource(shader, 1, srcs, &len);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		std::cout.write(src, len) << '\n';
		log_shader_err(shader);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint create_program(const std::string_view vtx_src, const std::string_view frag_src) {
	GLuint vtx_shader = create_shader(GL_VERTEX_SHADER, vtx_src.data(), vtx_src.size());
	if (!vtx_shader)
		return 0;
	defer { glDeleteShader(vtx_shader); };
	GLuint frag_shader = create_shader(GL_FRAGMENT_SHADER, frag_src.data(), frag_src.size());
	if (!frag_shader)
		return 0;
	defer { glDeleteShader(frag_shader); };
	GLuint prog = glCreateProgram();
	if (!prog)
		return 0;
	glAttachShader(prog, vtx_shader);
	glAttachShader(prog, frag_shader);
	glLinkProgram(prog);
	GLint status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		log_shader_err(prog);
		glDeleteProgram(prog); // pretty sure this doesn't call glDeleteShader on the shaders
		return 0;
	}
	return prog;
}

std::span<const int> bindings_of(int total_columns) {
	static const int bindings[MAX_COLUMN] = {
		SDL_SCANCODE_A,
		SDL_SCANCODE_S,
		SDL_SCANCODE_D,
		SDL_SCANCODE_F,
		SDL_SCANCODE_J,
		SDL_SCANCODE_K,
		SDL_SCANCODE_L,
		SDL_SCANCODE_SEMICOLON
	};
	int offset_from;
	switch (total_columns) {
	case 4:
	case 5:
		offset_from = 2;
		break;
	case 6:
	case 7:
		offset_from = 1;
		break;
	case 8:
		offset_from = 0;
		break;
	default:
		return std::span<int>();
	}
	return std::span(bindings + offset_from, total_columns);
}

// behold the magnificent 300+ line main function
int main(int argc, char **argv) {
	if (argc != 2) {
		const char *name = argv[0]; // the last element of argv is null
		if (!name)
			name = "beats";
		std::cout << "Usage: " << name << " [chart file]\n";
		return 1;
	}

	std::fstream in;
	in.open(argv[1]);
	if (!in.is_open()) {
		std::cout << "failed to open chart file\n";
		return -1;
	}
	Chart ch(600, 100, 8);
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
	//SDL_GL_SetSwapInterval(0); // diable vsync
	SDL_GL_SetSwapInterval(1); // enable vsync (SDL's default)
	GLenum gerr = glewInit(); // must be called after SDL_GL_CreateContext
	if (gerr != GLEW_OK) {
		std::cout << "failed to initialize GLEW: " << glewGetErrorString(gerr) << '\n';
		return -1;
	}

	char *c = argv[1];
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
	VLC::Media media(vlci, argv[1], VLC::Media::FromPath);
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
	glUniform3f(2, 0.486, 0.729, 0.815); // sets uniform "oColor"
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
		std::cout << "failed to vertex array objects\n";
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

	// TODO
	const uint64_t min_initial_delay = 0;
	uint64_t initial_delay = 0;
	Note *first = ch.first_note();
	if (!first) {
		std::cout << "that's a strange chart\n";
		return -1;
	}
	uint64_t first_time = first->timestamp;
	if (first_time < min_initial_delay)
		initial_delay = min_initial_delay - first_time;

	// the following variables are times in milliseconds
	uint64_t strike_timespan = 250; // pressing a key will result in a strike if the next note in the key's column is more than strike_timespan ms in the future or past
	//uint64_t perfect_threshhold = strike_timespan - strike_timespan / 8;
	//uint64_t great_threshhold = strike_timespan - strike_timespan / 5;
	//uint64_t good_threshhold = strike_timespan - strike_timespan / 3;
	uint64_t display_timespan = 650; // notes at the top of the screen will be display_timespan ms in the future
	uint64_t min_delay_per_frame = 5; // wait at least this long between each frame render (if vsync is on, this will not change things unless you want to draw slower than the refresh rate)
	int64_t audio_offset = -75; // given the delay of the headphones/speakers and the player's audio reaction time
	int64_t video_offset = -20; // given the delay of the keyboard and the player's visual reaction time
	// audio_offset and video_offset are optimal if the mean of all deltas
	// returned by close_note is 0.
	int64_t hold_note_unhold_offset = -110;

	uint64_t score = 0;
	uint64_t max_score = 0;

	SDL_RaiseWindow(win);

	// TODO
	if (false && initial_delay) {
		// VLC::MediaPlayer::play only calls libvlc_media_player_play, which
		// does not have documentation on its thread safety. So naturally, we
		// are being irresponsible and assuming it is thread safe. It likely
		// is, since libVLC is multithreaded under the hood.
		auto as = std::async(std::launch::async, [&]{
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
					/*
					if (note_score > perfect_threshhold)
						std::cout << "perfect ";
					else if (note_score > great_threshhold)
						std::cout << "great ";
					else if (note_score > good_threshhold)
						std::cout << "good ";
					else
						std::cout << "okay ";
					std::cout << note_score << '\n';
					*/
					found->active = false;
					score += note_score;
				} else {
					std::cout << "strike!\n";
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
						int64_t delta = song_offset + video_offset - (note->timestamp + note->hold_duration) + hold_note_unhold_offset;
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
				std::cout << "accuracy: " << static_cast<double>(score) / static_cast<double>(max_score) * 100.0 << "%\n";

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
