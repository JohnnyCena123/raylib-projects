#include "game.hpp"
#include "resource-manager.hpp"

fs::path Game::getSaveDir(bool portable) { return getDefaultSaveDir(); }
fs::path ResourceManager::getResourceDir(bool portable) {
	fs::path exeDir = GetApplicationDirectory();
	fs::path ret = exeDir;
	if (!portable) {
		bool found = false;
		while (ret.has_parent_path()) {
			std::array subdirOptions{
				fs::path{"."},
				fs::path{"share"},
				fs::path{"share"}/PROJECT_NAME,
				fs::path{"usr"}/"share"/PROJECT_NAME,
			};
			for (fs::path option : subdirOptions) {
				fs::path fullpath = ret/option;
				TraceLog(LOG_TRACE, "Checking %s", fullpath.string().c_str());
				if (isValidResourceDirPath(fullpath)) {
					found = true;
					break;
				}
			}
			if (found) break;
			ret = ret.parent_path();
		}
	}
	(void)verifyResourceDir(ret);
	return ret/"resources";
}
