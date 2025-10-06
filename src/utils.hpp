#pragma once
#include <filesystem>
#include <climits>
#include <raylib.h>
namespace fs = std::filesystem;

namespace utils {
	inline bool isValidResourceDirPath(fs::path const& dir) {
		fs::path resourceDir = dir/"resources";
		return DirectoryExists(resourceDir.string().c_str()) &&
			FileExists((resourceDir/"icon.png").string().c_str());
	};

	bool verifyResourceDir(fs::path const& dir);
	fs::path getResourceDir(bool portable);
	fs::path getDefaultResourceDir(bool portable);
	fs::path getDefaultSaveDir();
	fs::path getSaveDir(bool portable);

	std::string getHelpLauncherUri(std::string const& argv0);
}