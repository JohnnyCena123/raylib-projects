#include "game.hpp"
void Game::saveLogs() {
	std::time_t currentTime = std::time(0);
	std::tm* localTime = std::localtime(&currentTime);

	fs::path logsDir = m_saveDir/"logs";
	std::string logFilename = TextFormat("%04d.%02d.%02d_%02d-%02d-%02d.log",
		localTime->tm_year + 1900,
		localTime->tm_mon + 1,
		localTime->tm_mday,
		localTime->tm_hour,
		localTime->tm_min,
		localTime->tm_sec
	);
	fs::path logFilepath = logsDir/logFilename;
	if (!DirectoryExists(logsDir.string().c_str()))
		MakeDirectory(logsDir.string().c_str());
	bool saved = SaveFileText(logFilepath.string().c_str(), m_logs.str().c_str());
	if (m_hadWarning) {
		tinyfd_messageBox("Warning",
			(
				"Your latest run of " PROJECT_NAME " had logged a warning.\n"
				"logs can be found in " + logFilepath.string() + "."
			).c_str(), "ok", "warning", 1
		);
		if (saved) return;
		else tinyfd_messageBox("Failed", (
			"Failed to save logs to " + logFilepath.string() + ".\n"
			"Falling back to default save directory"
		).c_str(), "ok", "warning", 1);
		logsDir = [] -> fs::path {
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
		}()/"logs";
		if (!DirectoryExists(logsDir.string().c_str()))
			MakeDirectory(logsDir.string().c_str());
		logFilepath = logsDir/logFilename;

		if (!SaveFileText(logFilepath.string().c_str(), m_logs.str().c_str()) &&
			tinyfd_messageBox("Fail", (
				"Failed to save logs to " + logFilepath.string() + ".\n"
				"Do you want to copy them to clipboard?."
			).c_str(), "yesno", "warning", 1) &&
			!clipboard_set_text(m_cb, m_logs.str().c_str())
		) tinyfd_messageBox(
			"Fatal error",
			"Failed to copy logs to clipboard. You are out of luck...",
			"ok", "error", 1
		);
	}
}
