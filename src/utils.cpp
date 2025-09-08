#include <random>
#include "utils.hpp"
#include "basics.hpp"
std::random_device rd;
std::mt19937 mt{rd()};
Tile getRandomTile() {
	static std::uniform_int_distribution<int16_t> dist{0, GRID_SIZE - 1};
	return { static_cast<int8_t>(dist(mt)), static_cast<int8_t>(dist(mt)) };
}