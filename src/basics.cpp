#include <raylib.h>
#include "basics.hpp"

int screenWidth = 700;
int screenHeight = 500;

Path const resourceDir = [] -> Path {
	Path const exeDir = GetApplicationDirectory();
#ifdef _WIN32
	return exeDir;
#elif defined(__linux__)
	if (exeDir.string().starts_with("/usr") && 
		DirectoryExists("/usr/share/" PROJECT_NAME "/resources")
	) return Path{"/usr/share"}/PROJECT_NAME/"resources";
#endif
	return exeDir/"resources";
}();

Path saveDir = [] -> Path {
	Path ret = Path{GetApplicationDirectory()} / "save";
#ifdef _WIN32
	Path appData = std::getenv("APPDATA");
	ret = appData/PROJECT_NAME;
#elif defined(__linux__)
	Path homeDir = std::getenv("HOME");
	ret = homeDir/".local"/"share"/PROJECT_NAME;
#endif
	if (!DirectoryExists(ret.string().c_str())) 
		MakeDirectory(ret.string().c_str());
	return ret;
}();