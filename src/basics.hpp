// This file is for all sorts of constants and type aliases
#pragma once
#include <cstdint>
#include <array>
#include <cstdlib>
#include <string>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

using Tile = std::array<int, 2>;
using Apple = Tile;
using Direction = enum { None = 0, Up = 1, Down = 2, Left = 3, Right = 4 };

inline Tile constexpr START_POS = { 12, 10 };
inline int constexpr GRID_SIZE = 21;

inline size_t constexpr NEW_APPLE_INTERVAL = 10;
inline size_t constexpr SNAKE_START_LENGTH = 3;
inline float constexpr SNAKE_START_SPEED = 15.f;
inline float constexpr ACCELERATION_RATE = 1.f / 3.f; // the amount it speeds up by every time the score is incremented
inline float constexpr TILE_SIZE = 40.f;
inline float constexpr FREE_SPACE = 50.f;
inline float constexpr TILE_EDGE_SIZE = TILE_SIZE / 8;
inline float constexpr USED_TILE_SPACE = TILE_SIZE - TILE_EDGE_SIZE * 2;

using Grid = std::array<std::array<Rectangle, GRID_SIZE>, GRID_SIZE>;
inline Grid constexpr GRID = [] constexpr {
	Grid ret;
	for (size_t i = 0; i < GRID_SIZE; i++) {
		for (size_t j = 0; j < GRID_SIZE; j++) {
			ret[i][j] = {
				FREE_SPACE + i * TILE_SIZE,
				FREE_SPACE + j * TILE_SIZE,
				TILE_SIZE, TILE_SIZE,
			};
		}
	}
	return ret;
}();


inline float constexpr DEFAULT_SCREEN_SIZE = GRID_SIZE * TILE_SIZE + FREE_SPACE * 2;
inline float constexpr DEFAULT_DISPLAY_HEIGHT = 1080.f;

inline float constexpr RESTART_BUTTON_SIZE = 75.f;
inline struct {
	Vector2 origin;
	Vector2 center;
	float radius;
} constexpr RESTART_BUTTON_INFO = { {
		DEFAULT_SCREEN_SIZE / 2 - RESTART_BUTTON_SIZE,
		DEFAULT_SCREEN_SIZE / 2,
	}, {
		DEFAULT_SCREEN_SIZE / 2,
		DEFAULT_SCREEN_SIZE / 2 + RESTART_BUTTON_SIZE,
	}, RESTART_BUTTON_SIZE,
};
inline Color constexpr RESTART_BUTTON_OUTER_COLOR = { 0, 208, 51, 255 };
inline Color constexpr RESTART_BUTTON_INNER_COLOR = { 220, 220, 220, 255 };

inline std::string const SAVE_PATH = [] {
#ifdef _WIN32
	std::string appDataDir = std::getenv("APPDATA");
	std::string ret = appDataDir + "/Snake";
#else
	std::string ret = GetApplicationDirectory();
#endif
	return ret;
}();
inline char constexpr SAVE_FILE[] = "save.dat";
inline int8_t constexpr SAVE_DATA_XOR_KEY = 0x2f;
