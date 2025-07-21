#pragma once
#include <filesystem>

int constexpr START_SCREEN_WIDTH = 700;
int constexpr START_SCREEN_HEIGHT = 500;

namespace fs = std::filesystem;
extern fs::path const resourceDir;
extern fs::path saveDir;
