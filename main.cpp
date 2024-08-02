// under GPL-2.0

#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
//#include <SDL2/SDL_image.h>
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
	std::vector<Point> points;
	std::vector<uint32_t> indices;
private:
	std::vector<Note> notes;
	int total_columns; // the number of note columns in the chart
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
	std::pair<Note *, uint64_t> close_note(int column, uint64_t time, uint64_t threshhold);

	int width();

	int height();
};

Chart::Chart(uint32_t col_height, uint32_t note_width_, uint32_t note_height_) : points(512), indices(768), notes(0) {
	column_height = col_height + note_height_;
	note_width = note_width_;
	note_height = note_height_;
	note_index = 0;
	total_columns = 0;
}

#define READ(stream, dat, sz) stream.read(reinterpret_cast<char *>(dat), sz)

// expects a file with the following format:
// magic (8 bytes), version (4 bytes), note count (4 bytes)
// followed by note count number of
// timestamp (8 bytes), columns (4 bytes)
int Chart::deserialize(std::istream &is) {
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
		notes.resize(tmp.u_32);
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
			if (max_column > total_columns)
				total_columns = max_column;
			notes[i] = n;
		}
	} catch (const std::ios_base::failure &err) {
		return err.code().value();
	}
	return 0;
}

void Chart::draw(uint64_t t0, uint64_t t1) {
	points.clear(); // does not deallocate memory, though
	indices.clear();
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
			for (int j = 0; j < total_columns; j++) {
				if ((notes[i].columns >> j) & 1) {
					const int32_t x0 = note_width * j;
					const int32_t x1 = x0 + note_width;
					const int32_t y1 = column_height - ((column_height * (notes[i].timestamp - t0)) / (t1 - t0));
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
		}
		i++;
	}
}

std::pair<Note *, uint64_t> Chart::close_note(int column, uint64_t time, uint64_t threshhold) {
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
	return total_columns * note_width;
}

int Chart::height() {
	return column_height - note_height;
}

struct KeyStates {
	int data[SDL_NUM_SCANCODES];
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
	Chart ch(600, 100, 20);
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

	/*
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (!ren) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_DestroyRenderer(ren); };
	*/

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
	const char vtx_shader_src[] =
		"#version 460 core\n"
		"layout (location = 0) in ivec2 pix;\n"
		"void main() {\n"
		"	vec2 norm = 2.0 * vec2(pix) / vec2(400, 600) - 1.0;\n"
		"	gl_Position = vec4(norm, 0.0, 1.0);\n"
		"}";
	GLuint vtx_shader = load_shader(GL_VERTEX_SHADER, vtx_shader_src, sizeof(vtx_shader_src));
	if (!vtx_shader)
		return -1;
	defer { glDeleteShader(vtx_shader); };
	const char frag_color_src[] =
		"#version 460 core\n"
		"out vec4 frag_col;\n"
		"void main() {\n"
		"	frag_col = vec4(0.3, 0.5, 0.9, 1.0);\n"
		"}";
	GLuint frag_color = load_shader(GL_FRAGMENT_SHADER, frag_color_src, sizeof(frag_color_src));
	if (!frag_color)
		return -1;
	defer { glDeleteShader(frag_color); };
	GLuint note_prog = glCreateProgram();
	if (!note_prog)
		return -1;
	defer { glDeleteProgram(note_prog); }; // NOTE: these can be deleted after linking the program
	glAttachShader(note_prog, vtx_shader);
	glAttachShader(note_prog, frag_color);
	glLinkProgram(note_prog);
	GLint status;
	glGetProgramiv(note_prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		GLchar log[256];
		GLsizei log_len;
		glGetShaderInfoLog(note_prog, sizeof(log), &log_len, log);
		std::cout.write(log, log_len) << '\n';
		return -1;
	}

	GLuint vao;
	glGenVertexArrays(1, &vao);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to vertex array object\n";
		return -1;
	}
	GLuint buffers[2];
	glGenBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "failed to generate buffers\n";
		return -1;
	}
	defer { glDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers); };
	const GLuint vbo = buffers[0];
	const GLuint ebo = buffers[1];

	ch.draw(4500, 5000);
	for (const auto &p : ch.points) {
		std::cout << '(' << p.x << ", " << p.y << ")\n";
	}
	for (const auto &u : ch.indices) {
		std::cout << u << ", ";
	}
	std::cout << '\n';

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * ch.points.size(), ch.points.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * ch.indices.size(), ch.indices.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), 0); // why does GL_FLOAT work but GL_INT doesn't?
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*
	// not really an texture atlas yet, since we only have 1 texture :(
	// I'll probably make a class Atlas
	SDL_Texture *atlas = IMG_LoadTexture(ren, ASSETS("note.png"));
	if (!atlas) {
		std::cerr << "failed to load texture " ASSETS("note.png") "\n";
		return -1;
	}
	defer { SDL_DestroyTexture(atlas); };
	*/

	// "The audio device frequency is specified in Hz;"
	// "in modern times, 48000 is often a reasonable default." -SDL2_mixer wiki
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

	const uint64_t strike_timespan = 250; // pressing a key will result in a strike if the next note in the key's column is more than strike_timespan ms in the future
	const uint64_t display_timespan = 750; // notes at the top of the screen will be display_timespan ms in the future
	const uint64_t min_delay_per_frame = 5; // wait at least this long between each frame render
	int64_t audio_offset = -235;

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
				int col = ks.data[ev.key.keysym.scancode];
				if (!is_held && col) {
					auto [found, delta] = ch.close_note(col, song_offset, strike_timespan);
					if (found) {
						uint64_t score = strike_timespan - delta;
						/*
						if (score > strike_timespan)
							std::cout << "??? ";
						else if (score > strike_timespan - strike_timespan / 8)
							std::cout << "perfect ";
						*/
						std::cout << score << '\n';
						found->columns ^= col; // remove the note from its column to prevent it from being pressable and drawn
					} else {
						std::cout << "strike!\n";
					}
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
		glUseProgram(note_prog);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, ch.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0); // what's this for?
		SDL_GL_SwapWindow(win);
		/*
		SDL_RenderClear(ren);
		ch.draw(ren, atlas, song_offset, song_offset + display_timespan);
		SDL_RenderPresent(ren);
		*/

		uint64_t elapsed = SDL_GetTicks64() - start_frame;
		if (elapsed < min_delay_per_frame)
			SDL_Delay(min_delay_per_frame - elapsed);
	}
	return 0;
}
