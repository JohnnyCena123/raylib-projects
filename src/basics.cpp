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
		DirectoryExists("/usr/share/" APP_NAME "/resources")
	) return "/usr/share/" APP_NAME;
#endif
	return exeDir/"resources";
}();

Path const saveDir = [] -> Path {
	Path ret = Path{GetApplicationDirectory()} / "save";
#ifdef _WIN32
	Path appData = std::getenv("APPDATA");
	ret = appData/APP_NAME;
#elif defined(__linux__)
	Path homeDir = std::getenv("HOME");
	ret = homeDir/".local"/"share"/APP_NAME;
#endif
	if (!DirectoryExists(ret.string().c_str())) 
		MakeDirectory(ret.string().c_str());
	return ret;
}();