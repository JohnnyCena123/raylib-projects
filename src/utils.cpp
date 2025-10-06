#include <filesystem>
#include <iostream>
#include <string>
#include <sstream>
#include <raylib.h>
#ifdef PLATFORM_DESKTOP
	#include <tinyfiledialogs.h>
#endif
#include "utils.hpp"

fs::path utils::getDefaultResourceDir(bool portable) {
	fs::path ret = GetApplicationDirectory();
	(void)verifyResourceDir(ret);
	return ret/"resources";
}

static inline bool isValidResourceDirPath(fs::path const& dir) {
	fs::path resourceDir = dir/"resources";
	return DirectoryExists(resourceDir.string().c_str()) &&
		FileExists((resourceDir/"icon.png").string().c_str());
};

bool utils::verifyResourceDir(fs::path const& dir) {
	if (!isValidResourceDirPath(dir)) {
		TraceLog(LOG_ERROR, "Failed to locate resource dir; %s is not a valid parent directory.", dir.string().c_str());
		DESKTOP_ONLY(tinyfd_messageBox("Failure", (
			"Could not find the resource directory.\n"
			"Are you sure you downloaded the resources and extracted them to the right place?\n"
			"NOTE: the folder structure should look like this:\n"
			"/path/to/" PROJECT_NAME "/\n"
			"    |-- " PROJECT_NAME "\n"
			"    |-- libraries...\n"
			"    |-- resources/\n"
			"         |-- resources...\n"
			"additional info:\n" +
			dir.string() + " is not a valid parent directory for the resources."
			).c_str(), "ok", "error", 0
		));
		return false;
	} else return true;
}
fs::path utils::getDefaultSaveDir() { return fs::path{GetApplicationDirectory()}/"save"; }

static inline std::string encodeUri(const std::string &s) {
	std::ostringstream out{};
	for (unsigned char c : s) {
		if (
			(c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') ||
			 c == '-' || c == '_'  ||
			 c == '.' || c == '~'  ||
			 c == '/' || c == ':' 
		) out << c;
		else out << '%' << std::uppercase << std::hex << static_cast<int>(c) << std::nouppercase << std::dec;
	}
	return out.str();
}

std::string utils::getHelpLauncherUri(std::string const& argv0) {
	char constexpr SUFFIX[] = "-help";
	char constexpr WIN32_EXT[] = ".exe";

	size_t noDotExeLen = argv0.size();

#ifdef _WIN32
	noDotExeLen -= (sizeof(WIN32_EXT) - 1);
#endif

	std::string launcherFilepath = argv0.substr(0, noDotExeLen);
	launcherFilepath += SUFFIX;

#ifdef _WIN32
	launcherFilepath += WIN32_EXT;
#endif

	return TextFormat("file://%s", encodeUri(std::filesystem::absolute(launcherFilepath).string()).c_str());
}
