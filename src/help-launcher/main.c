#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	char const suffix[] = "-help";
	char const arg[] = "--help";
	char const win32Ext[] = ".exe";

	size_t noDotExeLen = strlen(argv[0]);
	noDotExeLen -= (sizeof(suffix) - 1);
	size_t cmdSize = noDotExeLen + (sizeof(" ") - 1) + (sizeof(arg) - 1);
#ifdef _WIN32
	cmdSize += (sizeof(win32Ext) - 1);
#endif

	char* cmd = calloc(cmdSize, sizeof(char));

	memcpy(cmd, argv[0], noDotExeLen);
#ifdef _WIN32
	strcat(cmd, win32Ext);
#endif
	strcat(cmd, " ");
	strcat(cmd, arg);

	int ret = system(cmd);
	unsigned char _[1];
	fread(_, sizeof(char), 1, stdin);
	free(cmd);
	return ret;
}
