#pragma once
#include <random>
#include <string>
#include <raylib.h>
#include "basics.hpp"
static inline Rectangle constexpr tileToRec(Tile const& tile) {
	return {
		FREE_SPACE + tile[0] * TILE_SIZE,
		FREE_SPACE + tile[1] * TILE_SIZE,
		TILE_SIZE, TILE_SIZE,

	};
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
static inline Direction constexpr vecToDirection(Tile vec) {
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