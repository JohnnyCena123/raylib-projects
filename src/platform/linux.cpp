#include "game.hpp"
#include "resource-manager.hpp"

fs::path Game::getSaveDir(bool portable) {
	fs::path ret = getDefaultSaveDir();
	if (!portable) {
		fs::path home = std::getenv("HOME");
		ret = home/".local"/"share"/PROJECT_NAME;
	}
	return ret;
}
fs::path ResourceManager::getResourceDir(bool portable) {
	fs::path exeDir = GetApplicationDirectory();
	fs::path ret = exeDir;
	TraceLog(LOG_DEBUG, "Looking for resource dir");
	if (!portable) {
		TraceLog(LOG_DEBUG, "Configuration: non-portable");
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
					ret = fullpath;
					break;
				}
			}
			if (found) break;
			ret = ret.parent_path();
		}
	} else TraceLog(LOG_DEBUG, "Configuration: portable");
	(void)verifyResourceDir(ret);
	return ret/"resources";
}
