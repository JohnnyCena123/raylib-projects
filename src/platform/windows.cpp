#include "game.hpp"
#include "resource-manager.hpp"

fs::path Game::getSaveDir(bool portable) {
	fs::path ret = getDefaultSaveDir();
	if (!portable) {
		fs::path appData = std::getenv("APPDATA");
		ret = appData/PROJECT_NAME;
	}
	return ret;
}
fs::path ResourceManager::getResourceDir(bool portable) { return getDefaultResourceDir(portable); }
