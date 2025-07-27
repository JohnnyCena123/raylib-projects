#pragma once

#include <sstream>
#include <libclipboard.h>
#include "basics.hpp"
#include "resource-manager.hpp"

class Game {
public:
	Game();
	Game(Game const&) = delete;
	Game(Game&&) = delete;
	~Game();

	void handleCli(int argc, char* argv[]);
	void init();
	void run();
	void deinit();

private:

	clipboard_c* m_cb;

	int m_traceLogLevel;
	bool m_silent;
	bool m_shouldSaveLogs;
	bool m_hadWarning;
	std::stringstream m_logs;

	int m_screenWidth = START_SCREEN_WIDTH;
	int m_screenHeight = START_SCREEN_HEIGHT;

	Texture2D m_dummyResource;
	float m_dummyResourceRotation;

	ResourceManager m_resourceManager{};

	void saveLogs();

};