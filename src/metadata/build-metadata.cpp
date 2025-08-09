#include "build-metadata.hpp"
std::string buildMetadata = 
	"Built for " CMAKE_SYSTEM " (" CMAKE_SYSTEM_NAME ")\n"
	"at " __TIME__ " on " __DATE__ "\n"
	"at the git commit hash: " GIT_COMMIT_SHORT " (" GIT_COMMIT ")\n"
	"using:\n"
	"  CMake version: " CMAKE_VERSION "\n"
	"  C Compiler: " C_COMPILER " version " C_COMPILER_VERSION "\n"
	"  C++ Compiler: " CXX_COMPILER " version " CXX_COMPILER_VERSION "\n"
	"  C Linker: " C_LINKER " version " C_LINKER_VERSION "\n"
	"  C++ Linker: " CXX_LINKER" version " CXX_LINKER_VERSION;