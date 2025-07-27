#include <cmath>
#include <raylib.h>
#include "game.hpp"
#include "basics.hpp"

Game::Game() : m_traceLogLevel(LOG_INFO), m_silent(false), m_shouldSaveLogs(false),
	m_hadWarning(false), m_logs(""), m_screenWidth(START_SCREEN_WIDTH), 
	m_screenHeight(START_SCREEN_HEIGHT), m_resourceManager() 
{ /* cant call initGame() here, handleCli() needs to be called first */ }

Game::~Game() { }

void Game::init() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(m_screenWidth, m_screenHeight, "Hello!");
	SetTargetFPS(60);

	m_resourceManager.init();
	if (!m_resourceManager.loadImage("app-icon", "icon.png"))
		TraceLog(LOG_WARNING, "Failed to load app icon");
	SetWindowIcon(m_resourceManager.getImage("app-icon"));

	if (!m_resourceManager.loadTexture("image", "image.png"))
		TraceLog(LOG_WARNING, "Failed to load dummy resource");
	m_dummyResource = m_resourceManager.getTexture("image");
}

void Game::run() {

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(SKYBLUE);

		m_dummyResourceRotation += GetFrameTime() * (70 * cos(GetTime() * 1.5) + 100.f);
		if (m_dummyResourceRotation > 360) m_dummyResourceRotation -= 360;

		float const scale = sin(GetTime()) + 2.f;
		Vector2 const position = {
			static_cast<float>(m_screenWidth - m_dummyResource.width * scale) / 2,
			static_cast<float>(m_screenHeight - m_dummyResource.height * scale) / 2,
		};

		Vector2 texSize = { static_cast<float>(m_dummyResource.width), static_cast<float>(m_dummyResource.height) };

		DrawTexturePro(m_dummyResource, { 0, 0, texSize.x, texSize.y }, { 
			m_screenWidth / 2.f, m_screenHeight / 2.f, 
			texSize.x * scale, texSize.y * scale 
		}, { texSize.x / 2, texSize.y / 2 }, m_dummyResourceRotation, WHITE);

		EndDrawing();

		m_screenWidth = GetScreenWidth();
		m_screenHeight = GetScreenHeight();
	}


}

void Game::deinit() {
	m_resourceManager.deinit();
	CloseWindow();
	if (m_shouldSaveLogs || m_hadWarning) saveLogs();
}
