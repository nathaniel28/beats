#include "keys.hpp"

bool Keystates::set(int scancode, bool change) {
	if (scancode >= SDL_NUM_SCANCODES)
		return false;
	bool res = pressed[scancode];
	pressed[scancode] = change;
	return res;
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
