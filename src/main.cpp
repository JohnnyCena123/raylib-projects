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
	int main();
	int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return main(); }
#endif
#include "game.hpp"

int main() {

	Game game;
	game.run();

	return 0;
}