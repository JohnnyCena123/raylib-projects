#pragma once

#include <raylib.h>
#ifndef IMGUI_OFF
	#include <imgui.h>
	#include <rlImGui.h>
	#define IMGUI_ONLY(...) __VA_ARGS__
#else
	#define IMGUI_ONLY(...)
#endif
#ifdef PLATFORM_DESKTOP
	#include <libclipboard.h>
	#include <tinyfiledialogs.h>
	#include <filesystem>
	#include <sstream>
	#include <optional>
	namespace fs = std::filesystem;
	#define DESKTOP_ONLY(...) __VA_ARGS__
#else
	#define DESKTOP_ONLY(...)
#endif
#include "resource-manager.hpp"

class Game {
public:
	Game();
	Game(Game const&) = delete;
	Game(Game&&) = delete;
	~Game();

	fs::path getResourceDir();

DESKTOP_ONLY(
	std::optional<int> handleCli(int argc, char* argv[]);
)
	void init();
	void run();
	void deinit();


private:

	fs::path m_resourceDir;

DESKTOP_ONLY(
	clipboard_c* m_cb;

	fs::path m_saveDir;

	bool m_portable;

	int m_traceLogLevel;
	bool m_silent;
	bool m_shouldSaveLogs;
	bool m_hadWarning;
	std::stringstream m_logs;
	void saveLogs();
)

	Vector2 m_screenSize;

	RenderTexture2D m_screen;

	Texture2D m_dummyResource;
	float m_dummyResourceRotation;

	Music m_bgMusic;

	ResourceManager m_resourceManager;

IMGUI_ONLY(
	void debugGUI();
)

};