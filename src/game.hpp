#pragma once

#include <filesystem>
#include <sstream>
#include <raylib.h>
#ifndef IMGUI_OFF
	#include <imgui.h>
	#include <rlImGui.h>
	#define IMGUI_ONLY(...) __VA_ARGS__
#else
	#define IMGUI_ONLY(...)
#endif
#include <libclipboard.h>
#include "resource-manager.hpp"

#include <raylib.h>

namespace fs = std::filesystem;

class Game {
public:
	Game();
	Game(Game const&) = delete;
	Game(Game&&) = delete;
	~Game();

	fs::path getrResourceDir();

	void handleCli(int argc, char* argv[]);
	void init();
	void run();
	void deinit();


private:

	clipboard_c* m_cb;

	fs::path m_resourceDir;
	fs::path m_saveDir;

	bool m_portable;

	int m_traceLogLevel;
	bool m_silent;
	bool m_shouldSaveLogs;
	bool m_hadWarning;
	std::stringstream m_logs;
	void saveLogs();

	Vector2 m_screenSize;

	RenderTexture2D m_screen;

	Texture2D m_dummyResource;
	float m_dummyResourceRotation;

	ResourceManager m_resourceManager;

	IMGUI_ONLY(
		void debugGUI();
	)

};