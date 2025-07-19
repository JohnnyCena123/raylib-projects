#pragma once
#include <filesystem>

int constexpr START_SCREEN_WIDTH = 700;
int constexpr START_SCREEN_HEIGHT = 500;

using Path = std::filesystem::path;
extern Path const resourceDir;
extern Path saveDir;
