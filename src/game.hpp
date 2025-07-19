#pragma once

#include "basics.hpp"
#include "resource-manager.hpp"

class Game {
public:
    Game();
	Game(Game const&) = delete;
	Game(Game&&) = delete;
    ~Game();

    void run();

private:

    int m_screenWidth = START_SCREEN_WIDTH;
    int m_screenHeight = START_SCREEN_HEIGHT;

	Texture2D m_dummyResource;
	float m_dummyResourceRotation;

    ResourceManager m_resourceManager{};

};