#include "utils.hpp"

fs::path utils::getDefaultResourceDir(bool portable) {
	fs::path ret = GetApplicationDirectory();
	(void)verifyResourceDir(ret);
	return ret/"resources";
}

bool utils::verifyResourceDir(fs::path dir) {
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
