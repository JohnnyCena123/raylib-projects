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
#include <raylib.h>

int constexpr SCREEN_WIDTH = 1800;
int constexpr SCREEN_HEIGHT = 900;

int main() {

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello!");

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(SKYBLUE);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}