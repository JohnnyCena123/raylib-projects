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
	namespace fs = std::filesystem;
#endif
#include "resource-manager.hpp"
	
#include <sstream>
#include <optional>

class Game {
public:
	Game() = delete;
	Game(int argc, char* argv[], std::optional<int>& exit); // no other way to get a return value from a ctor
	Game(Game const&) = delete;
	Game(Game&&) = delete;
	~Game();

	void run();

private:

	bool m_didInit;
	
	DESKTOP_ONLY(clipboard_c* m_cb);

NOT_IN_WEB(
	fs::path m_saveDir;
	static fs::path getDefaultSaveDir();
	static fs::path getSaveDir(bool portable);
)

	std::optional<int> handleCli(int argc, char* argv[]);

	bool m_portable;

	int m_traceLogLevel;
	bool m_silent;
	bool m_shouldSaveLogs;
	bool m_hadWarning;
	std::stringstream m_logs;
	void saveLogs();

	Vector2 m_screenSize;

	RenderTexture2D m_screen;

	Texture2D m_exampleResource;
	float m_exampleResourceRotation;

	Music m_bgMusic;

	ResourceManager m_resourceManager;

IMGUI_ONLY(
	void debugGUI();
)

};
