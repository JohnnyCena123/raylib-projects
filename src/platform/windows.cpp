#include <cstdio>
#include <iostream>
#define _WINGDI_
#define LF_FACESIZE 32
#define _WINUSER_
#define _APISETCONSOLEL3_
#define _WINSPOOL_
#define _OLE2_H_
#define _INC_COMMDLG
#define _IMM_SDK_DEFINED_
#define _PLAYSOUNDAPI_H_
#include <windows.h>
extern int main(int argc, char* argv[]);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	int argc;
	LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (!wargv) return 1;
	char** argv = new char*[argc];
	for (int i = 0; i < argc; ++i) {
		int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
		argv[i] = new char[len];
		WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], len, NULL, NULL);
	}
	bool allocedConsole = false;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (
			arg.starts_with('-') &&
			arg != "--silent" &&
			arg != "--verbose" &&
			arg != "--save-logs" &&
			arg != "--portable" &&
			!arg.starts_with("--resource-dir=") &&
			!arg.starts_with("--save-dir=") &&
			!arg.starts_with("--log-level=")
		) {
			if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
				AllocConsole();
				allocedConsole = true;
			}
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			freopen("CONIN$", "r", stdin);
			[] {
				HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
				if (hOut == INVALID_HANDLE_VALUE) return;
				DWORD dwMode = 0;
				if (!GetConsoleMode(hOut, &dwMode)) return;
				dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				SetConsoleMode(hOut, dwMode);
			}();
			break;
		}
	}
	int exit = main(argc, argv);
	if (allocedConsole) std::cin.get();
	else FreeConsole();
	for (int i = 0; i < argc; ++i) delete[] argv[i];
	delete[] argv;
	LocalFree(wargv);
	return exit;
}
