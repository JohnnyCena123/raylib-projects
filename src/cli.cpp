#include "platform/platforms.hpp"
#include "resource-manager.hpp"
#include <array>
#include <functional>
#include <raylib.h>
#include <regex>
#include <sstream>
#include <iostream>
#include <optional>
#include <string>
#ifdef PLATFORM_DESKTOP
	#include <tinyfiledialogs.h>
#endif
#include "metadata/build-metadata.hpp"
#include "game.hpp"

#define ERROR_MSG argv[0] << ": " << boldYellow << "error:" << reset << " "
#define HELP_SUGGESTION "use " << boldYellow << argv[0] << " --help" << reset << "for more info." << std::endl


std::optional<int> Game::handleCli(int argc, char* argv[]) {
	static std::string 
		reset = "\033[0m",
		hyperlink1 = "\033]8;;",
		hyperlink2 = "\033\\",
		bold = "\033[0;1m",
		blue = "\033[0;34m",
		cyan = "\033[0;36m",
		yellow = "\033[0;33m",
		boldYellow = "\033[1;33m",
		boldRed = "\033[1;31m",
		highlightedRed = "\033[0;30;41m";

	std::vector<std::string> args{};
	for (size_t i = 1; i < argc; i++) args.push_back(argv[i]);

	std::function<std::string(std::string, std::string)> hyperlink = [](std::string url, std::string text) -> std::string {
		return hyperlink1 + url + hyperlink2 + text + hyperlink1 + hyperlink2;
	};

	if (WEB_ONLY(true ||) std::find(args.begin(), args.end(), "--no-colors") != args.end()) {
		reset =
			hyperlink1 =
			hyperlink2 =
			bold =
			blue =
			cyan =
			yellow =
			boldYellow =
			boldRed =
			highlightedRed =
		"";
		hyperlink = [](std::string url, std::string text) -> std::string { return url; };
	}

	bool hasError = false;
	struct Option {
		std::optional<char> singleChar;
		std::string full;
		bool& v;
	};

	bool _;
	bool verbose = false;
	bool portable = false;
	bool printVersion = false;
	bool printHelp = false;
	bool noMetadata = false;
	bool dump = false;
	bool printDescription = false;
	bool printRepository = false;
	std::array options{
		Option{ 's', "--silent", m_silent },
		Option{ 'V', "--verbose", verbose },
		Option{ 'S', "--save-logs", m_shouldSaveLogs },
		Option{ 'p', "--portable", portable },
		Option{ 'v', "--version", printVersion },
		Option{ 'h', "--help", printHelp },
		Option{ 'u', "--usage", printHelp },
		Option{ std::nullopt, "--no-colors", _ },
		Option{ std::nullopt, "--no-metadata", noMetadata },
		Option{ std::nullopt, "--dump", dump },
		Option{ std::nullopt, "--description", printDescription },
		Option{ std::nullopt, "--repo", printRepository },
	};

	std::optional<std::string> rawLogLevel = std::nullopt;
	std::optional<fs::path> altResourceDir = std::nullopt;
	std::optional<fs::path> altSaveDir     = std::nullopt;
	for (size_t i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg.starts_with("--")) {
			bool found = false;
			static char constexpr ALT_RESOURCE_DIR_ARG[] = "--resource-dir=";
			static char constexpr ALT_SAVE_DIR_ARG[] = "--save-dir=";
			static char constexpr LOG_LEVEL_ARG[] = "--log-level=";
			
			if (arg.starts_with(LOG_LEVEL_ARG)) {
				rawLogLevel = arg.substr(sizeof(LOG_LEVEL_ARG) - 1, std::string::npos);
				found = true;
			} else if (arg.starts_with(ALT_RESOURCE_DIR_ARG)) {
				altResourceDir = arg.substr(sizeof(ALT_RESOURCE_DIR_ARG) - 1, std::string::npos);
				found = true;
			} else if (arg.starts_with(ALT_SAVE_DIR_ARG)) {
				altSaveDir = arg.substr(sizeof(ALT_SAVE_DIR_ARG) - 1, std::string::npos);
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

	static Game& _this = *this; // static for the tracelog callback to be able to access it
	TraceLogCallback traceLogCallback = [](int logType, const char* text, va_list args) {
		if (logType < _this.m_traceLogLevel) return;
		std::ostream* out = &std::cout; // has to be a pointer, references cant be reassigned
		if (logType >= LOG_WARNING) {
			_this.m_hadWarning = true;
			out = &std::cerr;
		}

		va_list argsCopy;
		va_copy(argsCopy, args);
		int size = std::vsnprintf(nullptr, 0, text, argsCopy);
		va_end(argsCopy);
		std::vector<char> buffer(size + 1);
		std::vsnprintf(buffer.data(), buffer.size(), text, args);

		std::time_t currentTime = std::time(0);
		std::tm* localTime = std::localtime(&currentTime);

		std::stringstream ss;
		ss << TextFormat("[%s%02d:%02d:%02d%s] ",
			bold.c_str(),
			localTime->tm_hour,
			localTime->tm_min,
			localTime->tm_sec,
			reset.c_str()
		);
		switch (logType) {
			case LOG_TRACE:     ss <<                   "TRACE: "   << reset; break;
			case LOG_DEBUG:     ss << blue           << "DEBUG: "   << reset; break;
			case LOG_INFO:      ss << cyan           << "INFO: "    << reset; break;
			case LOG_WARNING:   ss << yellow         << "WARNING: " << reset; break;
			case LOG_ERROR:     ss << boldRed        << "ERROR: "   << reset; break;
			case LOG_FATAL:     ss << highlightedRed << "FATAL: "   << reset; break;
			default: break;
		}
		ss << std::string(buffer.data());

		std::string noAnsi = std::regex_replace(ss.str(), std::regex{"\033\\[(\\d+|;)+m"}, "");

		if (!_this.m_silent) *out <<
			// avoid printing with colors to the web console, it wont work anyway
			NOT_IN_DESKTOP(withoutColors) DESKTOP_ONLY(ss.str()) << std::endl;
		_this.m_logs << noAnsi << std::endl;
	};
	SetTraceLogCallback(traceLogCallback);
	// im handling log levels myself
	SetTraceLogLevel(LOG_ALL);

	if (printRepository || printDescription || printVersion) {
		if (!printRepository) {
			if (dump) {
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
		if (dump) {
			std::cout << PROJECT_HOMEPAGE_URL << '\n';
			return 0;
		}
		std::cout << "The source code for '" PROJECT_NAME "' can be found in " <<
		boldYellow << hyperlink(PROJECT_HOMEPAGE_URL, "the GitHub Repository") << reset << ".\n";
		std::cout <<
			"This project is licensed under the LGPLv3.0 license - "
			"see " << boldYellow << hyperlink(PROJECT_HOMEPAGE_URL, "on GiHub") << reset << " for more details.\n";
		return 0;
	} else if (printHelp) {
		std::cout << "Usage: " << boldYellow << argv[0] << reset << " <options>\n"
			"    -s, --silent            --  disables all logging.\n"
			"    -V, --verbose           --  sets the log level to LOG_TRACE instead of LOG_INFO.\n"
			"    -S, --save-logs         --  saves logs to a file. unaffected by --silent.\n"
			"    -p, --portable          --  runs the application in portable mode.\n"
			"    -v, --version           --  prints the version and exits.\n"
			"    -h, --help              --  prints this help message and exits.\n"
			"    -u, --usage             --  same as --help.\n"
			"        --description       --  prints a general description of this app.\n"
			"        --no-metadata       --  dont print build metadata (build date & time, compiler, etc.)\n"
			"        --dump              --  used for --version, --description, and --repo.\n"
			"                                  dumps the minimal information needed to automate package metadata in GitHub Actions.\n"
			"        --repo              --  provides a link to the GitHub repository of the project.\n"
			"        --log-level=LEVEL   --  sets the log level to the specified input.\n"
			"                                  available log levels: all, trace, debug, info, warning, error, fatal, none\n"
			"        --resource-dir=DIR  --  sets a custom directory to use for resources."
			"                                  allows for relocating the resources directory without breaking the app.\n"
			"        --save-dir=DIR      --  sets a custom directory to use for save data. includes log files.\n";
		return 0;
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
					std::cout << "set log level to " << bold << #logLevel << reset << ".\n"; \
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
			std::cerr << ERROR_MSG << "invalid log level: " << bold << *rawLogLevel << reset << "\n"
				<< HELP_SUGGESTION;
			return 2;
		}
	}

#ifndef PLATFORM_WEB
	if (altSaveDir) {
		if (!DirectoryExists(altSaveDir->string().c_str())) {
			if (!MakeDirectory(altSaveDir->string().c_str())) {
				std::cerr << ERROR_MSG << "failed to create save directory: " << bold << altSaveDir->string() << reset << ".\n";
				return 2;
			}
		}
		m_saveDir = *altSaveDir;
	}
#endif
	if (altResourceDir) {
		if (!DirectoryExists(altResourceDir->string().c_str()))
			std::cerr << ERROR_MSG << "resource dir does not exist: " << bold << altResourceDir->string() << reset << ".\n";
		ResourceManager::s_resourceDir = *altResourceDir;
	} else ResourceManager::s_resourceDir = ResourceManager::getResourceDir(portable);
	return std::nullopt;
}
