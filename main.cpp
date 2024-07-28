#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// gonna need to wrap in OpenGL and freetype for font rendering...
// https://github.com/rougier/freetype-gl/tree/master/demos

// sdl resource:
// https://github.com/libSDL2pp/libSDL2pp-tutorial/blob/master/lesson00.cc

// see https://stackoverflow.com/questions/32432450
// thank you pmttavara!
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }
#define DEFER_(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__LINE__) = defer_dummy{} *[&]()

struct Note {
	uint64_t timestamp;
	//uint32_t hold_duration;
	uint32_t columns; // the nth bit is set if the nth column is used
};

class Chart {
//struct Chart { // temporary public everything
	std::vector<Note> notes;
	//std::vector<bool> hits; // equal in length to notes
	SDL_Rect note_bounds;
	int column_height;
	unsigned note_index;
	int total_columns;

	// file signature
	static const uint64_t magic = 0xF1E0007472616863;
	static const uint64_t version = 0;

public:
	Chart(int col_height, int note_width, int note_height);

	int deserialize(std::istream &);

	//int serialize(std::ostream &);

	// draw the notes from t0 to t1
	void draw(SDL_Renderer *, SDL_Texture *, uint64_t t0, uint64_t t1);

	// where column is the offset of the column to look for notes in, and
	// threshhold is how far in the future to search
	Note *next_note(int column, uint64_t threshhold);

	int width();

	int height();
};

Chart::Chart(int col_height, int note_width, int note_height) : notes(0) {
	column_height = col_height + note_height;
	note_bounds.w = note_width;
	note_bounds.h = note_height;
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
		for (unsigned i = 0; i < tmp.u_32; i++) {
			Note n;
			READ(is, &n.timestamp, sizeof(n.timestamp));
			READ(is, &n.columns, sizeof(n.columns));
			if (n.columns == 0)
				return -1;
			int max_column = 8 * sizeof(uint32_t) - std::countl_zero<uint32_t>(n.columns);
			if (max_column > total_columns)
				total_columns = max_column;
			notes[i] = n;
		}
	} catch (const std::ios_base::failure &err) {
		return err.code().value();
	}
	return 0;
}

void Chart::draw(SDL_Renderer *ren, SDL_Texture *tex, uint64_t t0, uint64_t t1) {
	unsigned i = note_index;
	//std::cerr << t0 << ' ' << t1 << ' ' << i << '\n';
	unsigned max = notes.size();
	while (i < max && notes[i].timestamp < t1) {
		if (notes[i].timestamp < t0) {
			note_index++; // next time, don't bother with this note
		} else {
			for (int j = 0; j < total_columns; j++) {
				if ((notes[i].columns >> j) & 1) {
					note_bounds.x = note_bounds.w * j;
					note_bounds.y = column_height - ((column_height * (notes[i].timestamp - t0)) / (t1 - t0)) - note_bounds.h;
					SDL_RenderCopy(ren, tex, nullptr, &note_bounds);
				}
			}
		}
		i++;
	}
}

Note *Chart::next_note(int column, uint64_t threshhold) {
	unsigned i = note_index;
	unsigned max = notes.size();
	while (i < max && notes[i].timestamp < threshhold) {
		if (notes[i].columns & column)
			return &notes[i];
		i++;
	}
	return nullptr;
}

int Chart::width() {
	return total_columns * note_bounds.w;
}

int Chart::height() {
	return column_height - note_bounds.h;
}

#define LOG_ERR() std::cerr << SDL_GetError() << '\n'

#define ASSETS(a) "./assets/" a

int main() {
	std::fstream in;
	in.open("charts/ba.chart");
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

	SDL_Window *win = SDL_CreateWindow("beats", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ch.width(), ch.height(), 0);
	if (!win) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_DestroyWindow(win); };

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (!ren) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_DestroyRenderer(ren); };

	// not really an texture atlas yet
	// I'll probably make a class Atlas
	SDL_Texture *atlas = IMG_LoadTexture(ren, ASSETS("note.png"));
	if (!atlas) {
		std::cerr << "failed to load texture " ASSETS("note.png") "\n";
		return -1;
	}
	defer { SDL_DestroyTexture(atlas); };

	uint64_t strike_timespan = 300;
	uint64_t display_timespan = 750;
	uint64_t min_delay_per_frame = 5;
	uint64_t start_chart = SDL_GetTicks64();
	while (1) {
		uint64_t start_frame = SDL_GetTicks64();
		uint64_t song_offset = start_frame - start_chart;

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_KEYDOWN) {
				int col = 0;
				switch (ev.key.keysym.sym) {
				case SDLK_d:
					col = 1;
					break;
				case SDLK_f:
					col = 2;
					break;
				case SDLK_j:
					col = 4;
					break;
				case SDLK_k:
					col = 8;
					break;
				}
				if (col) {
					Note *found = ch.next_note(col, song_offset + strike_timespan);
					if (found) {
						std::cout << strike_timespan - (found->timestamp - song_offset) << '\n';
						found->columns ^= col;
					} else {
						std::cout << "strike!\n";
					}
				}
			} else if (ev.type == SDL_QUIT) {
				return 0;
			}
		}

		SDL_RenderClear(ren);
		ch.draw(ren, atlas, song_offset, song_offset + display_timespan);
		SDL_RenderPresent(ren);

		uint64_t elapsed = SDL_GetTicks64() - start_frame;
		if (elapsed < min_delay_per_frame)
			SDL_Delay(min_delay_per_frame - elapsed);
	}
	return 0;
}
