#pragma once
#include <queue>
#include <string_view>

#include <raylib.h>
#ifndef CMAKE_RELEASE_BUILD
	#include <imgui.h>
	#include <rlImGui.h>
	#define DEBUG_ONLY(...) __VA_ARGS__
#else
	#define DEBUG_ONLY(...)
#endif

#include "save-data.hpp"
#include "snake.hpp"

// generic
class Game {

public:
	Game();
	~Game();

	bool run();

private:
	int m_stepCount;
	int m_score;
	float m_startSpeed;
	float m_speed;
	double m_roundStart;
	bool m_hasLost;
	bool m_isPaused;
	bool m_shouldRestart;
	bool m_restartButtonHovered;
	bool m_restartButtonHeld;
	bool m_isSaveDirty;
	
	std::queue<Direction> m_inputQueue;

	Snake m_snake;
	std::vector<Apple> m_apples;

	SaveData m_saveData;

	static SaveData loadSaveData(std::string_view saveFile);
	static void saveData(std::string_view saveFile, SaveData saveData);

	void reset();
	void checkDeath();
	void advanceScore();
	void updateSpeed(float newSpeed);
	void step();
	// returns: true - did step, false - did not
	bool update();
	// same here
	DEBUG_ONLY(bool debugGUI());
	void handleRestartButton(float resizeRatio);
	void draw() const;
	void drawOverlay() const;

};