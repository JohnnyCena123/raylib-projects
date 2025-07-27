#include <raylib.h>
#if defined(NO_TERMINAL_WINDOWS)
	// holy shit i hate raylib + windows
	// it compiles so shut up
	#define _WINGDI_
	#define _WINUSER_
	#define _APISETCONSOLEL3_
	#define _WINSPOOL_
	#define _OLE2_H_
	#define _INC_COMMDLG
	#define _IMM_SDK_DEFINED_
	#define _PLAYSOUNDAPI_H_
	#include <windows.h>
	int main(int argc, char* argv[]);
	int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
		// TODO: implement actual commandline arguments handling
		char buffer[MAX_PATH];
		(void)GetModuleFileName(NULL, buffer, MAX_PATH);
		char* argv[] = { buffer };
		return main(1, argv);
	}
#endif
#include "game.hpp"

int main(int argc, char* argv[]) {

	Game game{};
	game.handleCli(argc, argv);

	game.init();
	game.run();
	game.deinit();

	return 0;
}