#pragma once

#include <sstream>
#include "basics.hpp"
#include "resource-manager.hpp"

class Game {
public:
    Game(int argc, char* argv[]);
    Game() = delete;
	Game(Game const&) = delete;
	Game(Game&&) = delete;
    ~Game();

    void run();

private:

    void handleArgv(int argc, char* argv[]);

    bool m_shouldSaveLogs;
    std::stringstream m_logs;

    int m_screenWidth = START_SCREEN_WIDTH;
    int m_screenHeight = START_SCREEN_HEIGHT;

	Texture2D m_dummyResource;
	float m_dummyResourceRotation;

    ResourceManager m_resourceManager{};

};