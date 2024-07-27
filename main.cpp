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
};

class Chart {
	std::vector<Note> notes;

	// file signature
	static const uint64_t magic = 0x636861727400E0F1;
	static const uint64_t version = 0;

public:
	Chart() = default;

	int deserialize(std::istream &);

	//int serialize(std::ostream &);

};

#define LOG_ERR() std::cerr << SDL_GetError() << '\n'

int main() {
	int err = SDL_Init(SDL_INIT_VIDEO);
	if (err != 0) {
		LOG_ERR();
		return -1;
	}
	defer { SDL_Quit(); };

	SDL_Window *win = SDL_CreateWindow("test title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_INPUT_GRABBED);
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

	SDL_RenderClear(ren);
	SDL_RenderPresent(ren);

	SDL_Delay(5000);
	return 0;
}
