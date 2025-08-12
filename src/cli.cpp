#include <array>
#include <raylib.h>
#include <regex>
#include <sstream>
#include <iostream>
#include <optional>
#include <string>
#include <tinyfiledialogs.h>
#include "metadata/build-metadata.hpp"
#include "game.hpp"

#define ERROR_MSG argv[0] << ": \033[1;31merror:\033[0m "
#define HELP_SUGGESTION "use \033[1;33m" << argv[0] << " --help\033[0m for more info." << std::endl

std::optional<int> Game::handleCli(int argc, char* argv[]) {

	if (argc > 1) {
		bool hasError = false;
		struct Option {
			std::optional<char> singleChar;
			std::string full;
			bool& v;
		};
		bool noMetadata = false;
		bool minimalOutput = false;
		bool verbose = false;
		bool printVersion = false;
		bool printHelp = false;
		bool printDescription = false;
		bool printRepository = false;
		std::array options{
			Option{ std::nullopt, "--no-metadata", noMetadata },
			Option{ 'm', "--minimal-output", minimalOutput },
			Option{ 's', "--silent", m_silent },
			Option{ 'V', "--verbose", verbose },
			Option{ 'S', "--save-logs", m_shouldSaveLogs },
			Option{ 'p', "--portable", m_portable },
			Option{ 'v', "--version", printVersion },
			Option{ 'h', "--help", printHelp },
			Option{ 'u', "--usage", printHelp },
			Option{ 'd', "--description", printDescription },
			Option{ 'r', "--repo", printRepository },
		};
		
		std::optional<fs::path> altResourceir;
		std::optional<fs::path> altSaveDir;
		std::optional<std::string> rawLogLevel;
		for (size_t i = 1; i < argc; i++) {
			std::string arg = argv[i];
			if (arg.starts_with("--")) {
				bool found = false;
				static char constexpr ALT_RESOURCE_DIR_ARG[] = "--resource-dir=";
				static char constexpr ALT_SAVE_DIR_ARG[] = "--save-dir=";
				static char constexpr LOG_LEVEL_ARG[] = "--log-level=";
				if (arg.starts_with(ALT_RESOURCE_DIR_ARG)) {
					altResourceir = arg.substr(sizeof(ALT_RESOURCE_DIR_ARG) - 1, std::string::npos);
					found = true;
				} else if (arg.starts_with(ALT_SAVE_DIR_ARG)) {
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
						if (option.singleChar && arg[i] == option.singleChar) {
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

		if (hasError) return 2;

		if (printRepository || printDescription || printVersion) {
			if (!printRepository) {
				if (minimalOutput) {
					if (printVersion) std::cout << PROJECT_VERSION "\n";
					else if ( printDescription) std::cout << "Template project for raylib apps\n";
					return 0;
				}
				std::cout << PROJECT_NAME " version " PROJECT_VERSION ".\n";
				if (!noMetadata) std::cout << buildMetadata << "\n";
				if (printDescription) {
					std::cout <<
						"'" PROJECT_NAME "' is a template project. it is used to easily create\n"
						"and start new projects using raylib. it is not a full game or app.\n";
				}
			}
			if (minimalOutput) {
				std::cout << PROJECT_HOMEPAGE_URL << '\n';
				return 0;
			}
			std::cout << "The source code for '" PROJECT_NAME "' can be found in \033[1;33m"
				<< PROJECT_HOMEPAGE_URL << "\033[0m.\n";
			std::cout <<
				"This project is licensed under the LGPLv3.0 license - "
				"see \033[1;33m" << PROJECT_HOMEPAGE_URL << "/LICENSE\033[0m for more details.\n";
			return 0;
		} else if (printHelp) {
			std::cout << "Usage: \033[1;33m" << argv[0] << "\033[0m <options>\n"
				"        --no-metadata       --  dont print build metadata (build date & time, compiler, etc.)\n"
				"    -m, --minimal-output    --  used for --version, --description, and --repo. meant to automate package metadata in GitHub Actions.\n"
				"    -s, --silent            --  disables all logging.\n"
				"    -V, --verbose           --  sets the log level to LOG_TRACE instead of LOG_INFO.\n"
				"    -S, --save-logs         --  saves logs to a file. unaffected by --silent.\n"
				"    -p, --portable          --  runs the application in portable mode.\n"
				"    -v, --version           --  prints the version and exits.\n"
				"    -h, --help              --  prints this help message and exits.\n"
				"    -u, --usage             --  same as --help.\n"
				"    -d, --description       --  prints a general description of this app.\n"
				"    -r, --repo              --  provides a link to the GitHub repository of the project.\n"
				"        --resource-dir=DIR  --  sets a custom directory to use for resources. allows for relocating the resources directory without breaking the app.\n"
				"        --save-dir=DIR      --  sets a custom directory to use for save data. includes log files.\n"
				"        --log-level=LEVEL   --  sets the log level to the specified input.\n"
				"                                available log levels: all, trace, debug, info, warning, error, fatal, none\n";
			return 0;
		}

		if (altSaveDir) {
			if (!DirectoryExists(altSaveDir->string().c_str())) {
				if (!MakeDirectory(altSaveDir->string().c_str())) {
					std::cerr << ERROR_MSG << "failed to create save directory: " << altSaveDir->string() << ".\n";
					return 2;
				}
			}
			m_saveDir = *altSaveDir;
		}
		if (altResourceir) {
			if (!DirectoryExists(altResourceir->string().c_str()))
				std::cerr << ERROR_MSG << "resource dir does not exist: " << altResourceir->string() << ".\n";
			m_resourceDir = *altResourceir;
		}
		if (rawLogLevel) {
			if (rawLogLevel->empty()) {
				std::cerr << ERROR_MSG << "please provide a log level for --log-level.\n"
					"e.g. " << argv[0] << " --log-level=Debug\n"
					<< HELP_SUGGESTION;
					
				return 2;
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
				return 2;
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
	};
	SetTraceLogCallback(traceLogCallback);
	// im handling log levels myself
	SetTraceLogLevel(LOG_ALL);
	return std::nullopt;
}

void Game::saveLogs() {
	std::time_t currentTime = std::time(0);
	std::tm* localTime = std::localtime(&currentTime);

	fs::path logsDir = m_saveDir/"logs";
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
	m_logs = std::stringstream{std::regex_replace(m_logs.str(), std::regex{"\033\\[(\\d+|;)+m"}, "")};
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
