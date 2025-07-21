#include <raylib.h>
#include "basics.hpp"

int screenWidth = 700;
int screenHeight = 500;

fs::path const resourceDir = [] -> fs::path {
	fs::path const exeDir = GetApplicationDirectory();
#ifdef _WIN32
	return exeDir;
#elif defined(__linux__)
	if (exeDir.string().starts_with("/usr") && 
		DirectoryExists("/usr/share/" PROJECT_NAME "/resources")
	) return fs::path{"/usr/share"}/PROJECT_NAME/"resources";
#endif
	return exeDir/"resources";
}();

fs::path saveDir = [] -> fs::path {
	fs::path ret = fs::path{GetApplicationDirectory()} / "save";
#ifdef _WIN32
	fs::path appData = std::getenv("APPDATA");
	ret = appData/PROJECT_NAME;
#elif defined(__linux__)
	fs::path homeDir = std::getenv("HOME");
	ret = homeDir/".local"/"share"/PROJECT_NAME;
#endif
	if (!DirectoryExists(ret.string().c_str())) 
		MakeDirectory(ret.string().c_str());
	return ret;
}();