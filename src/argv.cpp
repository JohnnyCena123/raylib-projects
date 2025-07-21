#include <raylib.h>
#include <sstream>
#include <iostream>
#include <optional>
#include "game.hpp"

int traceLogLevel = LOG_INFO;

void Game::handleArgv(int argc, char* argv[]) {

	if (argc > 1) {
		bool hasError = false;
		struct Option {
			std::optional<char> singleChar;
			std::string full;
			bool& v;
		};
        static auto _this = this; // static for the tracelog callback to be able to access it
		bool minimalOutput = false;
		static bool silent = false; // here too
		bool verbose = false;
		bool saveLogs = false;
		bool printVersion = false;
		bool printHelp = false;
		bool printDescription = false;
		static std::vector<Option> const options{
			{ 'm', "--minimal-output", minimalOutput },
			{ 's', "--silent", silent },
			{ 'V', "--verbose", verbose },
			{ 'S', "--save-logs", saveLogs },
			{ 'v', "--version", printVersion },
			{ 'h', "--help", printHelp },
			{ 'u', "--usage", printHelp },
			{ 'd', "--desc", printDescription },
		};
		
		std::optional<fs::path> altSaveDir;
		for (size_t i = 1; i < argc; i++) {
			std::string arg = argv[i];
			if (arg.starts_with("--")) {
				bool found = false;
				static char constexpr ALT_SAVE_DIR_ARG[] = "--save-dir=";
				if (arg.starts_with(ALT_SAVE_DIR_ARG)) {
					altSaveDir = arg.substr(sizeof(ALT_SAVE_DIR_ARG) - 1, std::string::npos);
					found = true;
				} else for (auto const& option : options) {
					if (arg == option.full) {
						option.v = found = true;
						break;
					}
				}
				if (!found) {
					std::cerr << argv[0] << ": \033[31merror\033[0m: unrecognized option -- " << arg << std::endl;
					hasError = true;
				}
			} else if (arg.starts_with("-")) {
				for (size_t i = 1; i < arg.size(); i++) {
					bool found = false;
					for (auto const& option : options)  {
						if (arg[i] == option.singleChar) {
							option.v = found = true;
							break;
						}
					}
					if (!found) {
						std::cerr << argv[0] << ": \033[31merror\033[0m: unrecognized option -- -" << arg[i] << std::endl;
						hasError = true;
					}
				}
			} else {
				std::cerr << argv[0] << ": \033[31merror\033[0m: invalid argument -- " << arg << std::endl;
				hasError = true;
			}
		}
        /* 
		std::cout << "args found:\n";
		for (auto const& option : options) {
			if (option.v){
				std::cout << '\t';
				if (option.singleChar) std::cout << '-' << *option.singleChar << '/';
				std::cout << option.full << '\n';
			}
		}
 */
        if (verbose) traceLogLevel = LOG_TRACE;
        m_shouldSaveLogs = saveLogs;

		auto traceLogCallback = [](int logType, const char* text, va_list args) {
			static size_t constexpr MAX_TRACELOG_MSG_LENGTH = 512;
			if (logType < traceLogLevel) return;

			va_list argsCopy;
			va_copy(argsCopy, args);
			int size = std::vsnprintf(nullptr, 0, text, argsCopy);
			va_end(argsCopy);
			std::vector<char> buffer(size + 1);
			std::vsnprintf(buffer.data(), buffer.size(), text, args);

			std::time_t currentTime = std::time(0); 
			std::tm* localTime = std::localtime(&currentTime);

			std::stringstream ss;
			ss << "[ "
				<< localTime->tm_hour << ":"
				<< localTime->tm_min << ":"
				<< localTime->tm_sec << " ] ";
			switch (logType) {
				case LOG_TRACE: ss << "TRACE: "; break;
				case LOG_DEBUG: ss << "DEBUG: "; break;
				case LOG_INFO: ss << "INFO: "; break;
				case LOG_WARNING: ss << "WARNING: "; break;
				case LOG_ERROR: ss << "ERROR: "; break;
				case LOG_FATAL: ss << "FATAL: "; break;
				default: break;
			}
			ss << std::string(buffer.data());

			if (!silent) std::cout << ss.str() << std::endl;
			if (_this->m_shouldSaveLogs) _this->m_logs << ss.str() << std::endl;
            _this->m_logs.flush();
		};
		SetTraceLogCallback(traceLogCallback);
        
		if (printVersion) {
            if (minimalOutput) std::cout << PROJECT_VERSION "\n";
            else std::cout << "'" PROJECT_NAME "' is a template - it is not a distributed product.\n"
                "it does not follow the usual versioning scheme;\n"
                "but its version could be considered as " PROJECT_VERSION " .\n";
            exit(0);
        } else if (printHelp) {
            std::cout << "Usage: " << argv[0] << " <options>\n"
                "    -m, --minimal-output   --  used for --version and --description. meant to automate package metadata in GitHub Actions.\n"
                "    -s, --silent           --  disables all logging.\n"
                "    -V,  --verbose         --  sets the log level to LOG_TRACE instead of LOG_INFO.\n"
                "    -S,  --save-logs       --  saves logs to a file. unaffected by --silent.\n"
                "    -h, --help             --  prints this help message and exits.\n"
                "    -u, --usage            --  same as --help.\n"
                "    -d, --desc,            --  prints a general description of this app.\n"
                "        --save-dir=<dir>   --  sets a custom directory to use for save data. includes log files.\n";
            exit(0);
        } else if (printDescription) {
            if (minimalOutput) std::cout << "Template project for raylib apps\n";
            else std::cout << "'" PROJECT_NAME "' is a template project. it is used to easily create\n"
                "and start new projects using raylib. it is not a full game nor an app.\n";
            exit(0);
        }

        if (altSaveDir) {
            if (!DirectoryExists((*altSaveDir).string().c_str())) 
                MakeDirectory((*altSaveDir).string().c_str());
            saveDir = *altSaveDir;
        }

		if (hasError) exit(1);
	}

}