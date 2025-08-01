#pragma once
#include <filesystem>
#include <raylib.h>

inline Vector2 constexpr DEFAULT_SCREEN_SIZE = { 700, 500 };
inline Vector2 constexpr DEFAULT_DISPLAY_SIZE = { 1920, 1080 };

namespace fs = std::filesystem;
extern fs::path const resourceDir;
extern fs::path saveDir;
