#include "utils.hpp"
#include "basics.hpp"
#include <random>

std::random_device rd;
std::mt19937 mt{rd()};

Tile getRandomTile() {
	static std::uniform_int_distribution<int> dist{0, GRID_SIZE - 1};
	return { dist(mt), dist(mt) };
}