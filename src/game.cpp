#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <raylib.h>
#include "game.hpp"
#include "basics.hpp"
#include "resource-manager.hpp"

Game::Game(int argc, char* argv[], std::optional<int>& exit) : m_didInit(false),
#ifdef PLATFORM_DESKTOP
	m_cb(nullptr), m_saveDir(""), m_portable(
	#ifdef PORTABLE
		true
	#else
		FileExists((fs::path{GetApplicationDirectory()}/PORTABLE_INDICATOR_FILE).string().c_str())
	#endif
	),
#else
	m_portable(false),
#endif
	m_traceLogLevel(LOG_INFO), m_silent(false), m_shouldSaveLogs(false), m_hadWarning(false), m_logs(""),
	m_resourceManager(m_portable), m_screenSize(DEFAULT_SCREEN_SIZE), m_dummyResourceRotation(0.f) {

	if ((exit = handleCli(argc, argv))) return;

NOT_IN_WEB(
	m_saveDir = getSaveDir(m_portable);
)
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	InitWindow(m_screenSize.x, m_screenSize.y, "Hello!");
	SetTargetFPS(60);
	IMGUI_ONLY(rlImGuiSetup(true));

	InitAudioDevice();
	SetMasterVolume(.5f);

DESKTOP_ONLY(
	// i can only get the monitors size after the window is initialized :( this causes annoying problems
	int const monitorWidth = GetMonitorWidth(GetCurrentMonitor());
	int const monitorHeight = GetMonitorHeight(GetCurrentMonitor());
	float const resizeRatio = static_cast<float>(monitorHeight) / DEFAULT_DISPLAY_SIZE.y;
	Vector2 const newSize = { DEFAULT_SCREEN_SIZE.x * resizeRatio, DEFAULT_SCREEN_SIZE.y* resizeRatio };
	SetWindowSize(newSize.x, newSize.y);

	auto const windowPosX = GetWindowPosition().x;
	SetWindowPosition((monitorWidth - newSize.x) / 2, (monitorHeight - newSize.y) / 2);
)

	m_screen = LoadRenderTexture(DEFAULT_SCREEN_SIZE.x, DEFAULT_SCREEN_SIZE.y);

	m_resourceManager.init();
	if (!m_resourceManager.load<Image>("app-icon", "icon.png"))
		TraceLog(LOG_WARNING, "Failed to load app icon");
	SetWindowIcon(m_resourceManager.get<Image>("app-icon"));

	if (!m_resourceManager.load<Texture2D>("image", "image.png"))
		TraceLog(LOG_WARNING, "Failed to load dummy image");
	m_dummyResource = m_resourceManager.get<Texture2D>("image");

	// credit: https://sunixdev.itch.io/casual-music-pack
	if (!m_resourceManager.load<Music>("bg-music", "music-loop.mp3", [](Music& music) { music.looping = true; }))
		TraceLog(LOG_WARNING, "Failed to load background music");
	m_bgMusic = m_resourceManager.get<Music>("bg-music");

	m_didInit = true;
}

NOT_IN_WEB(fs::path Game::getDefaultSaveDir() { return fs::path{GetApplicationDirectory()}/"save"; })
void Game::run() {

	PlayMusicStream(m_bgMusic);

	while (!WindowShouldClose()) {

		UpdateMusicStream(m_bgMusic);

		if (IsWindowMaximized()) m_screenSize = {
			static_cast<float>(GetMonitorWidth(GetCurrentMonitor())),
			static_cast<float>(GetMonitorHeight(GetCurrentMonitor()))
		};
		else m_screenSize = {
			static_cast<float>(GetScreenWidth()),
			static_cast<float>(GetScreenHeight())
		};
		float resizeRatio = std::min(
			m_screenSize.x / DEFAULT_SCREEN_SIZE.x,
			m_screenSize.y / DEFAULT_SCREEN_SIZE.y
		);
		BeginTextureMode(m_screen); {
			ClearBackground(SKYBLUE);
			m_dummyResourceRotation += GetFrameTime() * (70 * cos(GetTime() * 1.5) + 100.f);
			if (m_dummyResourceRotation > 360) m_dummyResourceRotation -= 360;

			float const scale = sin(GetTime()) + 2.f;
			Vector2 texSize = { static_cast<float>(m_dummyResource.width), static_cast<float>(m_dummyResource.height) };
			DrawTexturePro(m_dummyResource, { 0, 0, texSize.x, texSize.y }, {
				DEFAULT_SCREEN_SIZE.x / 2, DEFAULT_SCREEN_SIZE.y / 2,
				texSize.x * scale, texSize.y * scale
			}, { texSize.x / 2, texSize.y / 2 }, m_dummyResourceRotation, WHITE);
		} EndTextureMode();
		BeginDrawing(); {
			Vector2 const actualScreenSize = {
				DEFAULT_SCREEN_SIZE.x * resizeRatio, 
				DEFAULT_SCREEN_SIZE.y * resizeRatio
			};
			ClearBackground(BLANK);
			DrawTexturePro(m_screen.texture,
				{ 0.f, 0.f, DEFAULT_SCREEN_SIZE.x, -DEFAULT_SCREEN_SIZE.y }, {
					(m_screenSize.x - actualScreenSize.x) / 2,
					(m_screenSize.y - actualScreenSize.y) / 2,
					actualScreenSize.x, actualScreenSize.y
				}, { 0.f, 0.f }, 0.f, WHITE
			);
			IMGUI_ONLY(
				rlImGuiBegin();
				debugGUI();
				rlImGuiEnd();
			)
		} EndDrawing();
	}
}

IMGUI_ONLY(void Game::debugGUI() {
	ImGui::Begin("Debug Window");
	ImGui::Text("Hello world!");
	ImGui::SliderFloat("rotation", &m_dummyResourceRotation, 0.f, 360.f);
	ImGui::End();
})

Game::~Game() {
	if (!m_didInit) return;
	m_resourceManager.deinit();
	UnloadRenderTexture(m_screen);
	CloseAudioDevice();
	IMGUI_ONLY(rlImGuiShutdown());
	CloseWindow();

DESKTOP_ONLY(
	if (m_shouldSaveLogs || m_hadWarning) saveLogs();

	clipboard_free(m_cb);
)
}
