#ifndef KEYS_HPP
#define KEYS_HPP

#include <array>
#include <span>

#include <SDL2/SDL.h>

#include "chart.hpp" // for MAX_COLUMN

#define ACT_NONE 0
#define ACT_COLUMN(n) (n + 1)
#define COLUMN_ACT_INDEX(n) (n - 1)
#define ACT_PAUSE (MAX_COLUMN + 1)

struct Keystates {
	std::array<int, SDL_NUM_SCANCODES> data;
	std::array<bool, SDL_NUM_SCANCODES> pressed;

	Keystates() = default;

	// sets the scancode to change and returns the previous value (true if the key is held)
	bool set(int scancode, bool change);
};

extern std::span<const int> bindings_of(int total_columns);

#endif
