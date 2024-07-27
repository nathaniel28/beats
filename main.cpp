//#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

// https://github.com/libSDL2pp/libSDL2pp-tutorial/blob/master/lesson00.cc
#include <SDL2/SDL.h>

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
public: // temporary everything public
	std::vector<Note> notes;
	//std::vector<bool> hits; // equal in length to notes
	SDL_Rect note_bounds;
	float column_height;
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
	uint64_t next_note_offset(int column, uint64_t threshhold);

};

Chart::Chart(int col_height, int note_width, int note_height) : notes(0) {
	column_height = static_cast<float>(col_height); // we need a float later
	note_bounds.w = note_width;
	note_bounds.h = note_height;
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
			notes[i] = n;
		}
	} catch (const std::ios_base::failure &err) {
		return err.code().value();
	}
	return 0;
}

void Chart::draw(SDL_Renderer *ren, SDL_Texture *tex, uint64_t t0, uint64_t t1) {
	unsigned i = note_index;
	unsigned max = notes.size();
	while (i < max && notes[i].timestamp < t1) {
		if (notes[i].timestamp < t0) {
			note_index++; // next time, don't bother with this note
			continue;
		}
		for (int j = 0; j < total_columns; j++) {
			if ((notes[i].columns >> j) & 1) {
				note_bounds.x = note_bounds.w * j;
				// there's gotta be a better way than messing
				// with int to float to int conversions
				float norm = static_cast<float>(notes[i].timestamp - t0) / static_cast<float>(t1 - t0);
				note_bounds.y = static_cast<int>(column_height * norm);
				SDL_RenderCopy(ren, tex, NULL, &note_bounds);
			}
		}
	}
}

uint64_t Chart::next_note_offset(int column, uint64_t threshhold) {
	unsigned i = note_index;
	unsigned max = notes.size();
	while (i < max && notes[i].timestamp < threshhold) {
		if (notes[i].columns & column)
			return notes[i].timestamp;
		i++;
	}
	return -1; // as unsigned
}

#define LOG_ERR() std::cerr << SDL_GetError() << '\n'

int main() {
	// temporary stuff
	std::fstream in;
	in.open("ba.chart");
	if (!in.is_open())
		return 2;
	Chart ch(800, 80, 20);
	if (!ch.deserialize(in)) {
		for (const auto &n : ch.notes) {
			std::cout << n.timestamp << ' ' << n.columns << '\n';
		}
	}

	return 0; // temporary

	/*
	int err = SDL_Init(SDL_INIT_VIDEO);
	if (err != 0) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_Quit(); };

	SDL_Window *win = SDL_CreateWindow("test title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
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

	uint64_t min_delay_per_frame = 10;
	while (1) {
		uint64_t start_frame = SDL_GetTicks64();

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			std::cout << ev.type << '\n';
			switch (ev.type) {
			case SDL_QUIT:
				return 0;
			}
		}
		SDL_RenderClear(ren);
		SDL_RenderPresent(ren);

		uint64_t elapsed = SDL_GetTicks64() - start_frame;
		if (elapsed < min_delay_per_frame)
			SDL_Delay(min_delay_per_frame - elapsed);
	}

	return 0;
	*/
}