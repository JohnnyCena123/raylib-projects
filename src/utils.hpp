#pragma once
#include <filesystem>
namespace fs = std::filesystem;

namespace utils {
	bool verifyResourceDir(fs::path dir);
	fs::path getResourceDir(bool portable);
	fs::path getDefaultResourceDir(bool portable);
	fs::path getDefaultSaveDir();
	fs::path getSaveDir(bool portable);
}