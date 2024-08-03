// under GPL-2.0

#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <GL/glew.h>
#include <GL/gl.h>

// gonna need to wrap in OpenGL and freetype for font rendering...
// https://github.com/rougier/freetype-gl/tree/master/demos

// sdl resource:
// https://sibras.github.io/OpenGL4-Tutorials/docs/Tutorials/01-Tutorial1

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
	uint64_t timestamp; // in milliseconds
	//uint32_t hold_duration;
	uint32_t columns; // a bitmask; the nth bit is set if the nth column is used, as the player hits a present column, the column is xor'd away, leaving the others columns untouched
};

class Chart {
public:
	std::vector<Point> points; // points of rectangles to draw notes
	std::vector<uint64_t> last_press; // last press of a column, millisecond offset from song start
	std::vector<uint32_t> indices; // indices for the EBO to help draw points
private:
	std::vector<Note> notes;
	unsigned note_index; // keep track of what note to start drawing from
	uint32_t column_height;
	uint32_t note_width;
	uint32_t note_height;

	// file signature
	static const uint64_t magic = 0xF1E0007472616863;
	static const uint64_t version = 0;

public:
	// calling the constructor does not make a Chart ready to use.
	// you must call deserialize next
	Chart(uint32_t col_height, uint32_t note_width, uint32_t note_height);

	// on success this function returns 0 and this Chart is safe to use
	// calls istream::exceptions
	int deserialize(std::istream &);

	//int serialize(std::ostream &);

	// draw the notes from t0 to t1, also updates note_index
	// future calls should be made with a greater t0 and t1
	void draw(uint64_t t0, uint64_t t1);

	// returns the closest *unpressed* note and the absolute time difference
	// between time and that note's timestamp, where column is the offset of
	// the column to look for notes in, and threshhold is how far in the
	// past/future to search
	std::pair<Note *, uint64_t> close_note(unsigned column, uint64_t time, uint64_t threshhold);

	int width();

	int height();

	int total_columns();
};

Chart::Chart(uint32_t col_height, uint32_t note_width_, uint32_t note_height_) : points(512), last_press(0), indices(768), notes(0) {
	column_height = col_height + note_height_;
	note_width = note_width_;
	note_height = note_height_;
	note_index = 0;
}

#define READ(stream, dat, sz) stream.read(reinterpret_cast<char *>(dat), sz)

// expects a file with the following format:
// magic (8 bytes), version (4 bytes), note count (4 bytes)
// followed by note count number of
// timestamp (8 bytes), columns (4 bytes)
int Chart::deserialize(std::istream &is) {
	int total_cols = 0;
	is.exceptions(std::ostream::failbit | std::ostream::badbit);
	try {
		// eventually, use endian.h to be extra standard, but for now,
		// assume little endian
		union {
			uint64_t u_64;
			uint32_t u_32;
		} tmp;
		READ(is, &tmp.u_64, sizeof(tmp.u_64));
		if (tmp.u_64 != Chart::magic)
			return -1;
		READ(is, &tmp.u_32, sizeof(tmp.u_32));
		if (tmp.u_32 != Chart::version)
			return -1;
		READ(is, &tmp.u_32, sizeof(tmp.u_32));
		notes.resize(tmp.u_32); // maybe use reserve instead?
		uint64_t last_timestamp = 0;
		for (unsigned i = 0; i < tmp.u_32; i++) {
			Note n;
			READ(is, &n.timestamp, sizeof(n.timestamp));
			if (n.timestamp < last_timestamp)
				return -1;
			last_timestamp = n.timestamp;
			READ(is, &n.columns, sizeof(n.columns));
			if (n.columns == 0)
				return -1;
			int max_column = 8 * sizeof(uint32_t) - std::countl_zero<uint32_t>(n.columns); // index of the last 1 in n.columns
			if (max_column > total_cols)
				total_cols = max_column;
			notes[i] = n;
		}
	} catch (const std::ios_base::failure &err) {
		return err.code().value();
	}
	last_press.resize(total_cols); // resize default initializes new elements
	return 0;
}

void Chart::draw(uint64_t t0, uint64_t t1) {
	points.clear(); // does not deallocate memory, though
	indices.clear(); // ditto
	unsigned i = note_index;
	//std::cerr << t0 << ' ' << t1 << ' ' << i << '\n';
	unsigned max = notes.size();
	while (i < max && notes[i].timestamp < t1) {
		if (notes[i].timestamp < t0) {
			if (notes[i].columns) {
				//std::cout << "miss!\n";
			}
			// we can do this because notes are kept ordered by time
			note_index++; // next time, don't bother with this note
		} else {
			int total_cols = total_columns();
			for (int j = 0; j < total_cols; j++) {
				if (!((notes[i].columns >> j) & 1))
					continue;
				const int32_t x0 = note_width * j;
				const int32_t x1 = x0 + note_width;
				const int32_t y1 = ((column_height * (notes[i].timestamp - t0)) / (t1 - t0));
				const int32_t y0 = static_cast<int32_t>(note_height) < y1 ? y1 - note_height : 0;
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
		}
		i++;
	}
}

std::pair<Note *, uint64_t> Chart::close_note(unsigned column, uint64_t time, uint64_t threshhold) {
	unsigned i = note_index;
	unsigned max = notes.size();
	Note *upper = nullptr;
	uint64_t upper_delta_t = 0;
	while (i < max && notes[i].timestamp < time + threshhold) {
		if (notes[i].columns & column) {
			upper = &notes[i];
			upper_delta_t = upper->timestamp - time;
			break;
		}
		i++;
	}
	i = note_index;
	while (i > 0 && notes[i].timestamp > time - threshhold) {
		if (notes[i].columns & column) {
			uint64_t lower_delta_t = time - notes[i].timestamp;
			if (!upper || upper_delta_t > lower_delta_t)
				return {&notes[i], lower_delta_t};
			break;
		}
		i--;
	}
	return {upper, upper_delta_t}; // possibly {nullptr, 0}
}

int Chart::width() {
	return total_columns() * note_width;
}

int Chart::height() {
	return column_height - note_height;
}

int Chart::total_columns() {
	return last_press.capacity();
}

struct KeyStates {
	unsigned data[SDL_NUM_SCANCODES];
	bool pressed[SDL_NUM_SCANCODES];

	KeyStates() = default; // RAII, those arrays are initialized!

	// sets the scancode to change and returns the previous value (true if the key is held)
	bool set(int scancode, bool change);
};

bool KeyStates::set(int scancode, bool change) {
	if (scancode >= SDL_NUM_SCANCODES)
		return false;
	bool res = pressed[scancode];
	pressed[scancode] = change;
	return res;
}

#define LOG_ERR() std::cerr << SDL_GetError() << '\n'

#define ASSETS(a) "./assets/" a

#define COLUMN(n) (1 << n)

GLuint load_shader(GLenum type, const GLchar *src, GLint len) {
	GLuint shader = glCreateShader(type);
	if (!shader)
		return 0;
	const char *const srcs[] = { src };
	glShaderSource(shader, 1, srcs, &len);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLchar log[256];
		GLsizei log_len;
		glGetShaderInfoLog(shader, sizeof(log), &log_len, log);
		std::cout.write(log, log_len) << '\n';
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint create_program(const std::string &vtx_src, const std::string &frag_src) {
	GLuint vtx_shader = load_shader(GL_VERTEX_SHADER, vtx_src.data(), vtx_src.size());
	if (!vtx_shader)
		return 0;
	defer { glDeleteShader(vtx_shader); };
	GLuint frag_shader = load_shader(GL_FRAGMENT_SHADER, frag_src.data(), frag_src.size());
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
		GLchar log[256];
		GLsizei log_len;
		glGetShaderInfoLog(prog, sizeof(log), &log_len, log);
		std::cout.write(log, log_len) << '\n';
		return 0;
	}
	return prog;
}

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
		std::cout << "failed to open file\n";
		return -1;
	}
	Chart ch(600, 100, 8);
	if (ch.deserialize(in)) {
		std::cout << "failed to deserialize file\n";
		return -1;
	}

	int err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (err != 0) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_Quit(); };

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_Window *win = SDL_CreateWindow("beats", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ch.width(), ch.height(), SDL_WINDOW_OPENGL);
	if (!win) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_DestroyWindow(win); };

	SDL_GLContext gl_ctx = SDL_GL_CreateContext(win);
	if (!gl_ctx) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_GL_DeleteContext(gl_ctx); };

	// TODO: move the following to a init OpenGL function of some kind
	GLenum gerr = glewInit(); // must be called after create context
	if (gerr != GLEW_OK) {
		std::cout << "failed to initialize GLEW: " << glewGetErrorString(gerr) << '\n';
		return -1;
	}
	glViewport(0, 0, ch.width(), ch.height());
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// TODO: move the following shader related stuff to another file
	// all other shaders should live there too
	//
	// TODO: screen size is hardcoded, pass in as a uniform or find it out,
	// without hardcoding it
	// this shader converts pixel coordinates (as ints)
	// to floats on the range [-1.0, 1.0] and colors them blue
	GLuint note_prog = create_program(
		"#version 460 core\n"
		"layout (location = 0) in ivec2 pix;\n"
		"void main() {\n"
		"	vec2 norm = 2.0 * vec2(pix) / vec2(400, 600) - 1.0;\n"
		"	gl_Position = vec4(norm, 0.0, 1.0);\n"
		"}"
		,
		"#version 460 core\n"
		"out vec4 frag_col;\n"
		"void main() {\n"
		"	frag_col = vec4(0.486, 0.729, 0.815, 1.0);\n"
		"}"
	);
	if (!note_prog)
		return -1;

	GLuint vao;
	glGenVertexArrays(1, &vao);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to vertex array object\n";
		return -1;
	}
	defer { glDeleteVertexArrays(1, &vao); };
	GLuint buffers[2];
	glGenBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to generate buffers\n";
		return -1;
	}
	defer { glDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers); };
	const GLuint vbo = buffers[0];
	const GLuint ebo = buffers[1];

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
	// tutorial also says to set GL_TEXTURE_MAG_FILTER and
	// GL_TEXTURE_MIN_FILTER to GL_LINEAR, but I think I can skip that
	glBindTexture(GL_TEXTURE_2D, 0); // why unbind if we're gonna rebind it ever time we render? idk but tutorial says so
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cbuf_tex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "failed to initialize framebuffer\n";
		return -1;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // I'm gonna need to go back and find out if all these unbinds are really required...

	GLuint postprocess_prog = create_program(
		"#version 460 core\n"
		"layout (location = 0) in vec2 aPos;\n"
		"layout (location = 1) in vec2 aTexCoords;\n"
		"out vec2 texCoords;\n"
		"void main() {\n"
		"	gl_Position = vec4(aPos, 0.0, 1.0);\n"
		"	texCoords = aTexCoords;\n"
		"}"
		,
		"#version 460 core\n"
		"out vec4 fragColor;\n"
		"in vec2 texCoords;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"	fragColor = texture(tex, texCoords);\n"
		"}"
	);
	if (!postprocess_prog)
		return -1;

	// "The audio device frequency is specified in Hz;"
	// "in modern times, 48000 is often a reasonable default."
	// -SDL2_mixer wiki
	err = Mix_OpenAudio(48000, AUDIO_F32SYS, 2, 4096);
	if (err != 0) {
		std::cerr << "failed to initialize audio\n";
		return -1;
	}
	defer { Mix_CloseAudio(); };

	char *c = argv[1];
	char *last_dot = c;
	while (*c) {
		if (*c == '.')
			last_dot = c;
		c++;
	}
	*last_dot = '\0';
	Mix_Music *track = Mix_LoadMUS(argv[1]);
	if (!track) {
		LOG_ERR();
		return -1;
	}
	defer { Mix_FreeMusic(track); };

	KeyStates ks;
	ks.data[SDL_SCANCODE_D] = COLUMN(0);
	ks.data[SDL_SCANCODE_F] = COLUMN(1);
	ks.data[SDL_SCANCODE_J] = COLUMN(2);
	ks.data[SDL_SCANCODE_K] = COLUMN(3);
	ks.data[SDL_SCANCODE_L] = COLUMN(4);

	// the following 4 variables are times in milliseconds
	uint64_t strike_timespan = 250; // pressing a key will result in a strike if the next note in the key's column is more than strike_timespan ms in the future
	uint64_t display_timespan = 750; // notes at the top of the screen will be display_timespan ms in the future
	uint64_t min_delay_per_frame = 5; // wait at least this long between each frame render
	int64_t audio_offset = -25;

	SDL_RaiseWindow(win);
	Mix_PlayMusic(track, 0);

	uint64_t start_chart = SDL_GetTicks64();
	while (1) {
		uint64_t start_frame = SDL_GetTicks64();
		uint64_t song_offset = start_frame - start_chart + audio_offset;

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_KEYDOWN:
			{
				// I need a new scope here because C++ throws a fit if you
				// declare variables after a label without one
				bool is_held = ks.set(ev.key.keysym.scancode, true);
				unsigned col = ks.data[ev.key.keysym.scancode];
				if (!col)
					break;
				unsigned col_index = std::countr_zero(col);
				if (col_index >= ch.last_press.size())
					break;
				ch.last_press[col_index] = song_offset;
				if (is_held) {
					// eventually, this will have code aside from break; so don't move it
					std::cout << "skipping held key\n";
					break;
				}
				auto [found, delta] = ch.close_note(col, song_offset, strike_timespan);
				if (found) {
					uint64_t score = strike_timespan - delta;
					/*
					if (score > strike_timespan)
						std::cout << "??? ";
					else if (score > strike_timespan - strike_timespan / 8)
						std::cout << "perfect ";
					*/
					//std::cout << score << '\n';
					found->columns ^= col; // remove the note from its column to prevent it from being pressable and drawn
					//std::cout << start_frame << ' ' << found->timestamp << '\n';
				} else {
					//std::cout << "strike!\n";
				}
				break;
			}
			case SDL_KEYUP:
				ks.set(ev.key.keysym.scancode, false);
				break;
			case SDL_QUIT:
				return 0;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		// ch.draw populates ch.points and ch.indices
		ch.draw(song_offset, song_offset + display_timespan);
		// now comes the OpenGL stuff I half understand
		// TODO: can any of this be called only once?
		// (before the render loop)
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * ch.points.size(), ch.points.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * ch.indices.size(), ch.indices.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), 0); // why does GL_FLOAT work but GL_INT doesn't?
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); // we don't need to do this if we make sure to bind buffers first, right?

		// more magic happens here
		glUseProgram(note_prog);
		glDrawElements(GL_TRIANGLES, ch.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0); // what's this for? resetting state?
		SDL_GL_SwapWindow(win);

		uint64_t elapsed = SDL_GetTicks64() - start_frame;
		if (elapsed < min_delay_per_frame)
			SDL_Delay(min_delay_per_frame - elapsed);
	}
	return 0;
}
