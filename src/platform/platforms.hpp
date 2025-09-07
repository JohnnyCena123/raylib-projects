#pragma once
#ifdef PLATFORM_DESKTOP
	#define DESKTOP_ONLY(...) __VA_ARGS__
	#define NOT_IN_DESKTOP(...)
#else
	#define DESKTOP_ONLY(...)
	#define NOT_IN_DESKTOP(...) __VA_ARGS__
#endif

#ifdef PLATFORM_WEB
	#define WEB_ONLY(...) __VA_ARGS__
	#define NOT_IN_WEB(...)
#else
	#define WEB_ONLY(...)
	#define NOT_IN_WEB(...) __VA_ARGS__
#endif


// TODO: add android support
/*
#ifdef PLATFORM_ANDROID
	#define ANDROID_ONLY(...) __VA_ARGS__
	#define NOT_IN_ANDROID(...)
#else
	#define ANDROID_ONLY(...)
	#define NOT_IN_ANDROID(...) __VA_ARGS__
#endif
*/
