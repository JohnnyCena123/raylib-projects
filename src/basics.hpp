// This file is for all sorts of constants and type aliases
#pragma once
#include <cstdint>
#include <array>
#include <cstdlib>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
using Tile = std::array<int8_t, 2>;
using Apple = Tile;
using Direction = enum { None = 0, Up = 1, Down = 2, Left = 3, Right = 4 };
inline Tile constexpr START_POS = { 12, 10 };
inline int constexpr GRID_SIZE = 21;
inline size_t constexpr NEW_APPLE_INTERVAL = 10;
inline size_t constexpr SNAKE_START_LENGTH = 3;
inline float constexpr SNAKE_START_SPEED = 7.5f;
inline float constexpr ACCELERATION_RATE = 1.f / 3.f; // the amount it speeds up by every time the score is incremented
inline float constexpr TILE_SIZE = 40.f;
inline float constexpr FREE_SPACE = 50.f;
inline float constexpr TILE_EDGE_SIZE = TILE_SIZE / 8;
inline float constexpr USED_TILE_SPACE = TILE_SIZE - TILE_EDGE_SIZE * 2;
inline Vector2 constexpr DEFAULT_SCREEN_SIZE = {
    GRID_SIZE * TILE_SIZE + FREE_SPACE * 2,
    GRID_SIZE * TILE_SIZE + FREE_SPACE * 2
};
inline Vector2 constexpr DEFAULT_DISPLAY_SIZE = { 1920, 1080 };
inline float constexpr RESTART_BUTTON_SIZE = 75.f;
inline struct {
	Vector2 origin;
	Vector2 center;
	float radius;
} constexpr RESTART_BUTTON_INFO = { {
		DEFAULT_SCREEN_SIZE.x / 2 - RESTART_BUTTON_SIZE,
		DEFAULT_SCREEN_SIZE.y / 2,
	}, {
		DEFAULT_SCREEN_SIZE.x / 2,
		DEFAULT_SCREEN_SIZE.y / 2 + RESTART_BUTTON_SIZE,
	}, RESTART_BUTTON_SIZE,
};
inline Color constexpr RESTART_BUTTON_OUTER_COLOR = { 0, 208, 51, 255 };
inline Color constexpr RESTART_BUTTON_INNER_COLOR = { 220, 220, 220, 255 };
inline char constexpr SAVE_FILE[] = "save.dat";
inline int8_t constexpr SAVE_DATA_XOR_KEY = 0x2f;
