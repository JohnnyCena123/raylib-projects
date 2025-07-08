#pragma once

#include <random>
#include <string>
#include <raylib.h>

#include "basics.hpp"

static inline Rectangle constexpr& recFromIndices(Tile const& indices, Grid& grid) {
	return grid[indices[0]][indices[1]];
}
static inline Rectangle const constexpr& recFromIndices(Tile const& indices, Grid const& grid) {
	return grid[indices[0]][indices[1]];
}

static inline std::string constexpr directionToString(Direction direction) {
	switch (direction) {
		case Up:    return "up";
		case Down:  return "down";
		case Left:  return "left";
		case Right: return "right";
		default:    return "none";
	}
	return std::to_string(direction) + " (error)";
}

static inline std::array<int, 2> constexpr directionToVec(Direction direction) {
	switch (direction) {
		case Up:    return { 0, -1 };
		case Down:  return { 0,  1 };
		case Left:  return { -1, 0 };
		case Right: return { 1,  0 };
		default:    return { 0,  0 };
	}
}
static inline Direction constexpr vecToDirection(std::array<int, 2> vec) {
	switch (vec[0]) {
		case GRID_SIZE - 1:
		case 1: return Left;
		case -(GRID_SIZE - 1):
		case -1: return Right;
		case 0: switch (vec[1]) {
			case GRID_SIZE - 1:
			case 1: return Up;
			case -(GRID_SIZE - 1):
			case -1: return Down;
		} break;
	}
	return None;
}

extern std::random_device rd;
extern std::mt19937 mt;
Tile getRandomTile();