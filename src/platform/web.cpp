#include "game.hpp"
#include "resource-manager.hpp"
#include <raylib.h>

void Game::saveLogs() { TraceLog(LOG_INFO, "Refusing to save logs on web"); }
fs::path ResourceManager::getResourceDir(bool portable) { return getDefaultResourceDir(portable); }