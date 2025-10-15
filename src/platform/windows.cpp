#include "utils.hpp"

fs::path utils::getSaveDir(bool portable) {
	fs::path ret = getDefaultSaveDir();
	if (!portable) {
		fs::path appData = std::getenv("APPDATA");
		ret = appData/PROJECT_NAME;
	}
	return ret;
}
fs::path utils::getResourceDir(bool portable) { return getDefaultResourceDir(portable); }
