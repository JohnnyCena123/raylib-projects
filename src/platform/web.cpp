#include "utils.hpp"
#include <raylib.h>

void Game::saveLogs() { TraceLog(LOG_INFO, "Refusing to save logs on web"); }
fs::path utils::getResourceDir(bool portable) { return getDefaultResourceDir(portable); }
fs::path utils::getSaveDir    (bool portable) { return getDefaultSaveDir(); }