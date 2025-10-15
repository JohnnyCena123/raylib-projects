#pragma once
#ifdef IMGUI_EXPORTING
	#ifdef _WIN32
		#define IMGUI_API __declspec(dllexport)
	#else
		#define IMGUI_API __attribute__((visibility("default")))
	#endif
#elif defined(_WIN32)
	#define IMGUI_API __declspec(dllimport)
#endif