#include <array>
#include <raylib.h>
#include <sstream>
#include <iostream>
#include <optional>
#include <string>
#include <tinyfiledialogs.h>
#include "game.hpp"

#define ERROR_MSG argv[0] << ": \033[1;31merror:\033[0m "
#define HELP_SUGGESTION "use \033[1;33m" << argv[0] << " --help\033[0m for more info." << std::endl

void Game::handleCli(int argc, char* argv[]) {

	if (argc > 1) {
		bool hasError = false;
		struct Option {
			char singleChar;
			std::string full;
			bool& v;
		};
		bool minimalOutput = false;
		bool verbose = false;
		bool printVersion = false;
		bool printHelp = false;
		bool printDescription = false;
		bool printRepository = false;
		std::array options{
			Option{ 'm', "--minimal-output", minimalOutput },
			Option{ 's', "--silent", m_silent },
			Option{ 'V', "--verbose", verbose },
			Option{ 'S', "--save-logs", m_shouldSaveLogs },
			Option{ 'v', "--version", printVersion },
			Option{ 'h', "--help", printHelp },
			Option{ 'u', "--usage", printHelp },
			Option{ 'd', "--description", printDescription },
			Option{ 'r', "--repo", printRepository },
		};
		
		std::optional<fs::path> altSaveDir;
		std::optional<std::string> rawLogLevel;
		for (size_t i = 1; i < argc; i++) {
			std::string arg = argv[i];
			if (arg.starts_with("--")) {
				bool found = false;
				static char constexpr ALT_SAVE_DIR_ARG[] = "--save-dir=";
				static char constexpr LOG_LEVEL_ARG[] = "--log-level=";
				if (arg.starts_with(ALT_SAVE_DIR_ARG)) {
					altSaveDir = arg.substr(sizeof(ALT_SAVE_DIR_ARG) - 1, std::string::npos);
					found = true;
				} else if (arg.starts_with(LOG_LEVEL_ARG)) {
					rawLogLevel = arg.substr(sizeof(ALT_SAVE_DIR_ARG), std::string::npos);
					found = true;
				} else for (auto const& option : options) {
					if (arg == option.full) {
						option.v = found = true;
						break;
					}
				}
				if (!found) {
					std::cerr << ERROR_MSG << "unrecognized option -- " << arg << "\n"
						<< HELP_SUGGESTION;
					hasError = true;
				}
			} else if (arg.starts_with("-")) {
				for (size_t i = 1; i < arg.size(); i++) {
					bool found = false;
					for (auto& option : options)  {
						if (arg[i] == option.singleChar) {
							option.v = found = true;
							break;
						}
					}
					if (!found) {
						std::cerr << ERROR_MSG << "unrecognized option -- -" << arg[i] << "\n"
							<< HELP_SUGGESTION;
						hasError = true;
					}
				}
			} else {
				std::cerr << ERROR_MSG << "invalid argument -- " << arg << "\n"
					<< HELP_SUGGESTION;
				hasError = true;
			}
		}

		if (hasError) exit(2);

		if (printVersion) {
			if (minimalOutput) std::cout << PROJECT_VERSION "\n";
			else std::cout << "'" PROJECT_NAME "' is a template - it is not a distributed product.\n"
				"it does not follow the usual versioning scheme;\n"
				"but its version could be considered as " PROJECT_VERSION " .\n";
			exit(0);
		} else if (printHelp) {
			std::cout << "Usage: \033" << argv[0] << " <options>\n"
				"    -m, --minimal-output   --  used for --version, --description, and --repo. meant to automate package metadata in GitHub Actions.\n"
				"    -s, --silent           --  disables all logging.\n"
				"    -V, --verbose          --  sets the log level to LOG_TRACE instead of LOG_INFO.\n"
				"    -S, --save-logs        --  saves logs to a file. unaffected by --silent.\n"
				"    -v, --version          --  prints the version and exits.\n"
				"    -h, --help             --  prints this help message and exits.\n"
				"    -u, --usage            --  same as --help.\n"
				"    -d, --description             --  prints a general description of this app.\n"
				"    -r, --repo             --  provides a link to the GitHub repository of the project.\n"
				"        --save-dir=DIR     --  sets a custom directory to use for save data. includes log files.\n"
				"        --log-level=LEVEL  --  sets the log level to the specified input.\n"
				"                               available log levels: all, trace, debug, info, warning, error, fatal, none\n";
			exit(0);
		} else if (printDescription) {
			if (minimalOutput) std::cout << "Template project for raylib apps";
			else std::cout <<
				"'" PROJECT_NAME "' is a template project. it is used to easily create\n"
				"and start new projects using raylib. it is not a full game or app.\n";
			exit(0);
		} else if (printRepository) {
			if (!minimalOutput) std::cout << "The repository for the project " PROJECT_NAME " can be found in \033[1;33m";
			std::cout << PROJECT_HOMEPAGE_URL;
			if (!minimalOutput) std::cout << "\033[0m.";
			std::cout << std::endl;
			exit(0);
		}

		if (altSaveDir) {
			if (!DirectoryExists(altSaveDir->string().c_str())) {
				if (!MakeDirectory(altSaveDir->string().c_str())) {
					std::cerr << ERROR_MSG << "failed to create save directory: " << altSaveDir->string() << ".\n";
					exit(2);
				}
			}
			saveDir = *altSaveDir;
		}
		if (rawLogLevel) {
			if (rawLogLevel->empty()) {
				std::cerr << ERROR_MSG << "please provide a log level for --log-level.\n"
					"e.g. " << argv[0] << " --log-level=Debug\n"
					<< HELP_SUGGESTION;
					
				exit(2);
			}
			bool invalidLogLevel = false;
			if (rawLogLevel->size() < 3) invalidLogLevel = true;
			else {
				std::string usedLogLevel = TextToUpper(rawLogLevel->c_str());
				usedLogLevel = usedLogLevel.starts_with("LOG_") ? usedLogLevel : "LOG_" + usedLogLevel;

			#define LOG_LEVEL_CASE(logLevel) \
				case #logLevel[4]: { \
					if (usedLogLevel == #logLevel) { \
						m_traceLogLevel = logLevel; \
						std::cout << "set log level to \033[1m" #logLevel "\033[0m.\n"; \
					} else invalidLogLevel = true; \
				} break
				switch (usedLogLevel[4]) {
					LOG_LEVEL_CASE(LOG_ALL);
					LOG_LEVEL_CASE(LOG_TRACE);
					LOG_LEVEL_CASE(LOG_DEBUG);
					LOG_LEVEL_CASE(LOG_INFO);
					LOG_LEVEL_CASE(LOG_WARNING);
					LOG_LEVEL_CASE(LOG_ERROR);
					LOG_LEVEL_CASE(LOG_FATAL);
					LOG_LEVEL_CASE(LOG_NONE);
					default: invalidLogLevel = true; break;
				}
			}
			if (invalidLogLevel) {
				std::cerr << ERROR_MSG << "invalid log level: " << *rawLogLevel << "\n"
					<< HELP_SUGGESTION;
				exit(2);
			}
		}

	}

	static Game& _this = *this; // static for the tracelog callback to be able to access it
	TraceLogCallback traceLogCallback = [](int logType, const char* text, va_list args) {
		if (logType < _this.m_traceLogLevel) return;
		if (logType >= LOG_WARNING) _this.m_hadWarning = true;

		va_list argsCopy;
		va_copy(argsCopy, args);
		int size = std::vsnprintf(nullptr, 0, text, argsCopy);
		va_end(argsCopy);
		std::vector<char> buffer(size + 1);
		std::vsnprintf(buffer.data(), buffer.size(), text, args);

		std::time_t currentTime = std::time(0);
		std::tm* localTime = std::localtime(&currentTime);

		std::stringstream ss;
		ss << TextFormat("[\033[0;1m%02d:%02d:%02d\033[0m] ",
			localTime->tm_hour,
			localTime->tm_min,
			localTime->tm_sec
		);
		switch (logType) {
			case LOG_TRACE:     ss << "TRACE: "; break;
			case LOG_DEBUG:     ss << "\033[0;34mDEBUG:\033[0m "; break;
			case LOG_INFO:      ss << "\033[0;36mINFO:\033[0m "; break;
			case LOG_WARNING:   ss << "\033[0;33mWARNING:\033[0m "; break;
			case LOG_ERROR:     ss << "\033[1;31mERROR:\033[0m "; break;
			case LOG_FATAL:     ss << "\033[0;30;41mFATAL:\033[0m "; break;
			default: break;
		}
		ss << std::string(buffer.data());

		if (!_this.m_silent) std::cout << ss.str() << std::endl;
		_this.m_logs << ss.str() << std::endl;
		if (logType == LOG_FATAL) {
			_this.m_traceLogLevel = LOG_NONE;
			_this.saveLogs();
			exit(EXIT_FAILURE);
		}
	};
	SetTraceLogCallback(traceLogCallback);
	// im handling log levels myself
	SetTraceLogLevel(LOG_ALL);
}

void Game::saveLogs() {
	std::time_t currentTime = std::time(0);
	std::tm* localTime = std::localtime(&currentTime);

	fs::path logsDir = saveDir/"logs";
	std::string logFilename = TextFormat("%04d.%02d.%02d-%02d:%02d:%02d.log",
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
		if (!saved) tinyfd_messageBox("Failed", (
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
