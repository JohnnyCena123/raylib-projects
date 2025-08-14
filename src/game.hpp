#pragma once
#include <queue>
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
	#include <sstream>
	#include <optional>
	#include <filesystem>
	namespace fs = std::filesystem;
	#define DESKTOP_ONLY(...) __VA_ARGS__
#else
	#define DESKTOP_ONLY(...)
#endif
#include "resource-manager.hpp"
#include "snake.hpp"
#include "save-data.hpp"
class Game {
public:
	Game();
	Game(Game const&) = delete;
	Game(Game&&) = delete;
	~Game();
	fs::path getResourceDir();
	DESKTOP_ONLY(std::optional<int> handleCli(int argc, char* argv[]));
	void init();
	bool run();
	void deinit();
private:
	fs::path m_resourceDir;
DESKTOP_ONLY(
	fs::path m_saveDir;
	clipboard_c* m_cb;
	bool m_portable;
	int m_traceLogLevel;
	bool m_silent;
	bool m_shouldSaveLogs;
	bool m_hadWarning;
	std::stringstream m_logs;
)
	Vector2 m_screenSize;
	RenderTexture2D m_screen;
	Music m_bgMusic;
	ResourceManager m_resourceManager;
	bool m_muted;
	float m_masterVolume; 
	int m_stepCount;
	int m_score;
	float m_startSpeed;
	float m_speed;
	float m_timeSinceStep;
	bool m_hasLost;
	bool m_isPaused;
	bool m_shouldRestart;
	bool m_restartButtonHeld;
	bool m_isSaveDirty;
	std::queue<Direction> m_inputQueue;
	Snake m_snake;
	std::vector<Apple> m_apples;
	SaveData m_saveData;
DESKTOP_ONLY(
	void saveLogs();
	void loadSaveData(fs::path saveFile);
	void saveData(fs::path saveFile);
)
	void reset();
	void checkDeath();
	void advanceScore();
	void step();
	void update();
	void handleRestartButton(float resizeRatio);
	void draw() const;
IMGUI_ONLY(
	void debugGUI();
)
	void drawOverlay() const;
};