#include <algorithm>
#include <raylib.h>
#include <string>
#include "game.hpp"
#include "imgui.h"
#include "resource-manager.hpp"
#include "utils.hpp"
#include "save-data.hpp"
#include "snake.hpp"
#include "basics.hpp"
Game::Game() :
#ifdef PLATFORM_DESKTOP
	m_cb(nullptr), m_resourceDir(""), m_saveDir(""), m_portable(
#ifdef PORTABLE
		true
	#else
		FileExists((fs::path{GetApplicationDirectory()}/PORTABLE_INDICATOR_FILE).string().c_str())
	#endif
	),
	m_traceLogLevel(LOG_INFO), m_silent(false), m_shouldSaveLogs(false),
	m_hadWarning(false), m_logs(""),
#endif
	m_screenSize(DEFAULT_SCREEN_SIZE), m_resourceManager(*this), m_muted(false),
	m_masterVolume(.5f), m_stepCount(0), m_score(0), m_startSpeed(SNAKE_START_SPEED),
	m_speed(m_startSpeed), m_timeSinceStep(0.f), m_hasLost(false), m_isPaused(false),
	m_shouldRestart(false), m_restartButtonHeld(false), m_isSaveDirty(false),
	m_inputQueue(), m_snake(SNAKE_START_LENGTH, *this), m_apples(),
	m_saveData(0)
{ /* cant call init() here, handleCli() needs to be called first */ }
Game::~Game() { }
fs::path Game::getResourceDir() { return m_resourceDir; }
static inline bool isValidResourceDirPath(fs::path dir) {
	fs::path resourceDir = dir/"resources";
	return DirectoryExists(resourceDir.string().c_str()) &&
		FileExists((resourceDir/"icon.png").string().c_str());
};
void Game::init() {
	m_resourceDir = [&] -> fs::path {
	#ifdef PLATFORM_WEB
		fs::path ret = ".";
	#else
		fs::path exeDir = GetApplicationDirectory();
		fs::path ret = exeDir;
		if (!m_portable) {
		#ifdef __linux__
			fs::path testedDir = exeDir;
			std::optional<fs::path> finalPath;
			while (testedDir.has_parent_path()) {
				std::array subdirOptions{
					fs::path{"."},
					fs::path{"share"},
					fs::path{"share"}/PROJECT_NAME,
					fs::path{"usr"}/"share"/PROJECT_NAME
				};
				for (fs::path option : subdirOptions) {
					fs::path fullpath = testedDir/option;
					TraceLog(LOG_TRACE, "Testing %s", fullpath.string().c_str());
					if (isValidResourceDirPath(fullpath)) {
						TraceLog(LOG_INFO, "Detected %s as the parent for the resources directory", fullpath.string().c_str());
						finalPath = fullpath;
						break;
					}
				}
				if (finalPath) break;
				testedDir = testedDir.parent_path();
			}
			ret = *finalPath;
		#endif
		}
		if (!isValidResourceDirPath(ret)) {
			DESKTOP_ONLY(tinyfd_messageBox("Failure", (
				"Could not find the resource directory.\n"
				"Are you sure you downloaded the resources and extracted them to the right place?\n"
				"NOTE: the folder structure should look like this:\n"
				"/path/to/" PROJECT_NAME "/\n"
				"    |-- " PROJECT_NAME "\n"
				"    |-- libraries...\n"
				"    |-- resources/\n"
				"         |-- resources...\n"
				"additional info:\n" +
				ret.string() + " is not a valid parent directory for the resources."
				).c_str(), "ok", "error", 0
			));
			TraceLog(LOG_ERROR, "Failed to locate resource dir; %s is not a valid parent directory.", ret.string().c_str());
		}
	#endif
		return ret/"resources";
	}();
#ifdef PLATFORM_DESKTOP
	m_saveDir = [&] -> fs::path {
		fs::path ret = fs::path{GetApplicationDirectory()}/"save";
		if (!m_portable) {
			#ifdef _WIN32
				fs::path appData = std::getenv("APPDATA");
				ret = appData/PROJECT_NAME;
			#elif defined(__linux__)
				fs::path homeDir = std::getenv("HOME");
				ret = homeDir/".local"/"share"/PROJECT_NAME;
			#endif
		}
		if (!DirectoryExists(ret.string().c_str())) MakeDirectory(ret.string().c_str());
		return ret;
	}();
	m_cb = clipboard_new(nullptr);
#endif
	DESKTOP_ONLY(loadSaveData(SAVE_FILE));
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(DEFAULT_SCREEN_SIZE.x, DEFAULT_SCREEN_SIZE.y, "Snake");
	SetTargetFPS(60);
	IMGUI_ONLY(rlImGuiSetup(true));
	rlSetLineWidth(3.f);
	InitAudioDevice();
	SetMasterVolume(.5f);
DESKTOP_ONLY(
	// i can only get the monitors size after the window is initialized :( this causes annoying problems
	int const monitorWidth = GetMonitorWidth(GetCurrentMonitor());
	int const monitorHeight = GetMonitorHeight(GetCurrentMonitor());
	float const resizeRatio = static_cast<float>(monitorHeight) / DEFAULT_DISPLAY_SIZE.y;
	Vector2 const newSize = { DEFAULT_SCREEN_SIZE.x * resizeRatio, DEFAULT_SCREEN_SIZE.y* resizeRatio };
	SetWindowSize(newSize.x, newSize.y);
	float const windowPosX = GetWindowPosition().x;
	SetWindowPosition((monitorWidth - newSize.x) / 2, (monitorHeight - newSize.y) / 2);
)
	m_screen = LoadRenderTexture(DEFAULT_SCREEN_SIZE.x, DEFAULT_SCREEN_SIZE.y);
	m_resourceManager.init();
DESKTOP_ONLY(
	if (!m_resourceManager.loadImage("app-icon", "icon.png"))
		TraceLog(LOG_WARNING, "Failed to load app icon");
	SetWindowIcon(m_resourceManager.getImage("app-icon"));
)
	LoadImageCallback resizer = [](Image& image) {
		ImageResize(&image, FREE_SPACE - 20.f, FREE_SPACE - 20.f);
	};
	m_resourceManager.loadTexture("apple", "apple.png", resizer);
	m_resourceManager.loadTexture("trophy", "trophy.png", resizer);
	// credit: https://sunixdev.itch.io/casual-music-pack
	if (!m_resourceManager.loadMusic("bg-music", "music-loop.mp3", [](Music& music) { music.looping = true; }))
		TraceLog(LOG_WARNING, "Failed to load background music");
	m_bgMusic = m_resourceManager.getMusic("bg-music");
	PlayMusicStream(m_bgMusic);
}
DESKTOP_ONLY(
void Game::loadSaveData(fs::path saveFile) {
	fs::path realPath = m_saveDir/saveFile;
	if (FileExists(realPath.string().c_str())) {
		std::string rawFileData = LoadFileText(realPath.string().c_str());
		std::string decoded;
		{
			int base64DecodedSize;
			char const* base64DecodedCStr = reinterpret_cast<char const*>(DecodeDataBase64(rawFileData.c_str(), &base64DecodedSize));
			if (base64DecodedCStr != nullptr) {
				std::string base64Decoded{base64DecodedCStr, static_cast<size_t>(base64DecodedSize)};
				for (char const _char : base64Decoded) decoded.push_back(_char ^ SAVE_DATA_XOR_KEY);
			} else {
				TraceLog(LOG_ERROR, "Failed to decode save data from base64.");
				TraceLog(LOG_INFO, "[DEBUG]");
				TraceLog(LOG_INFO, "Raw corrupted base64 data:\n%s", rawFileData.c_str());
				return;
			}
		}
		std::vector<std::string> linesVec{};
		std::istringstream stream{decoded};
		std::string line;
		while (std::getline(stream, line, '\n')) linesVec.push_back(line);
		std::string final;
		for (std::string const& str : linesVec) {
			if (!str.empty() && str[0] != '#') {
				final = str;
				break;
			}
		}
		if (final.size() > 3) {
			TraceLog(LOG_ERROR, "Don't cheat! %s is not normal.", final.c_str());
			m_saveData.highScore = -200;
			return;
		}
		try {
			m_saveData.highScore = std::stoi(final);
		} catch (std::invalid_argument const&) {
			TraceLog(LOG_ERROR, "Error parsing save data from file.");
			TraceLog(LOG_INFO, "[DEBUG]");
			TraceLog(LOG_INFO, "Got:\n%s", decoded.c_str());
			TraceLog(LOG_INFO, "Parsed:\n%s", final.c_str());
		}
		TraceLog(LOG_INFO, "Successfully loaded save data from %s", realPath.string().c_str());
	} else TraceLog(LOG_INFO, "Save File does not exist at %s, defaulting high-score to 0.", realPath.string().c_str());
	return;
}
void Game::saveData(fs::path saveFile) {
	std::string raw = TextFormat(
		"# This file was automatically generated by the Snake game.\n"
		"# All lines starting with a # will be ignored.\n"
		"# Any change made to this file will be discarded on the next time you start the game.\n"
		"# Although if you're already here you might deserve that little bit of extra score.\n"
		"# (reading the code doesn't count, you lame cheater)\n"
		"%i", m_saveData
	);
	std::string xored; // xored as in something that had gone through the xor process. https://en.wikipedia.org/wiki/Bitwise_operation#XOR
	for (char const _char : raw) xored.push_back(_char ^ SAVE_DATA_XOR_KEY);
	int _;
	std::string encoded = EncodeDataBase64(reinterpret_cast<unsigned char const*>(xored.c_str()), xored.size(), &_);
	if (!SaveFileText((m_saveDir/saveFile).string().c_str(), encoded.c_str()))
		TraceLog(LOG_ERROR, "Failed to save the following save data: %s\n", encoded.c_str());
}
)
void Game::reset() {
	m_stepCount
		= m_score
		= 0;
	m_speed = m_startSpeed;
	m_timeSinceStep = 0.f;
	// m_roundStart = GetTime();
	m_hasLost
		= m_isPaused
		= m_shouldRestart
		= m_restartButtonHeld
		= false;
	while (!m_inputQueue.empty()) m_inputQueue.pop(); // if only std::queue::clear() existed...
	m_snake.reset();
	Apple firstApple;
	do firstApple = getRandomTile();
	while (std::find(m_snake.m_tiles.begin(), m_snake.m_tiles.end(), firstApple) != m_snake.m_tiles.end());
	m_apples.clear();
	m_apples.push_back(firstApple);
}
bool Game::run() {
	reset();
	bool isWindowMaximized = false;
	while (!WindowShouldClose()) {
		SetMasterVolume(m_muted ? 0.f : m_masterVolume);
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
		update();
		BeginTextureMode(m_screen); {
			ClearBackground(BLANK);
			draw();
			handleRestartButton(resizeRatio);
		} EndTextureMode();
		BeginDrawing();
			Vector2 const actualScreenSize = DEFAULT_SCREEN_SIZE * resizeRatio;
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
		EndDrawing();
		if (IsKeyPressed(KEY_M)) m_muted ^= true;
		if (IsKeyPressed(KEY_Q)) return false;
		if (IsKeyPressed(KEY_R) || m_shouldRestart) return true;
	}
	return false;
}
void Game::checkDeath() {
	int n = 0;
	for (Tile const& tile : m_snake.m_tiles) {
		Tile const& nextTile = m_snake.getNextTile();
		Tile const& tail =  m_snake.m_tiles.front();
		if (tile == nextTile && tile != tail) {
			m_hasLost = true;
			if (m_snake.m_tiles.size() < 4) TraceLog(LOG_WARNING, "Oops! you are not supposed to die this short!");
			return;
		}
	}
}

void Game::advanceScore() {
	m_score++;
	m_saveData.highScore = std::max(m_saveData.highScore, m_score);
}

void Game::step() {
	m_snake.m_tiles.push_back(m_snake.getNextTile());

	bool pop = true;
	for (Apple& apple : m_apples) {
		if (CheckCollisionRecs(
			recFromIndices(m_snake.m_tiles.back(), GRID),
			recFromIndices(apple, GRID))
		) {
			pop = false;
			Apple newApple = apple;
			do newApple = getRandomTile();
			while (
				std::find(m_snake.m_tiles.begin(), m_snake.m_tiles.end(), newApple) != m_snake.m_tiles.end() ||
				std::find(m_apples.begin(), m_apples.end(), newApple) != m_apples.end()
			);
			apple = newApple;
			advanceScore();
			m_speed += ACCELERATION_RATE;
		}
	}
	if (pop) m_snake.m_tiles.pop_front();
}

void Game::update() {
	auto processInput = [&](Direction direction) {
		if (m_inputQueue.size() > 5) return;
		bool changeDirection = false;
		Direction lastInput = m_inputQueue.empty() ? m_snake.m_direction : m_inputQueue.back();
		switch (direction) {
			case Up:    if (lastInput != Down  && lastInput != Up   ) changeDirection = true; break;
			case Down:  if (lastInput != Up    && lastInput != Down ) changeDirection = true; break;
			case Left:  if (lastInput != Right && lastInput != Left ) changeDirection = true; break;
			case Right: if (lastInput != Left  && lastInput != Right) changeDirection = true; break;
			default: break;
		}
		if (changeDirection) m_inputQueue.push(direction);
	};
	{
		if (!m_hasLost) {
			switch(GetKeyPressed()) {
				case KEY_K:
				case KEY_W:
				case KEY_UP: processInput(Up); break;
				case KEY_J:
				case KEY_S:
				case KEY_DOWN: processInput(Down); break;
				case KEY_H:
				case KEY_A:
				case KEY_LEFT: processInput(Left); break;
				case KEY_L:
				case KEY_D:
				case KEY_RIGHT: processInput(Right); break;
			}
			if (IsKeyPressed(KEY_SPACE)) m_isPaused = !m_isPaused;
			if (m_score >= m_apples.size() * NEW_APPLE_INTERVAL) {
				Apple newApple;
				do newApple = getRandomTile();
				while (
					std::find(m_snake.m_tiles.begin(), m_snake.m_tiles.end(), newApple) != m_snake.m_tiles.end() ||
					std::find(m_apples.begin(), m_apples.end(), newApple) != m_apples.end()
				);
				m_apples.push_back(newApple);
			}
		}
		m_timeSinceStep += GetFrameTime();
		if (m_timeSinceStep * m_speed > 1) {
			m_stepCount++;
			m_timeSinceStep -= 1 / m_speed;
			if (!m_isPaused && !m_hasLost) {
				Direction prev = m_snake.m_direction;
				if (!m_inputQueue.empty()) {
					m_snake.m_direction = m_inputQueue.front();
					m_inputQueue.pop();
				}
				checkDeath();
				if (m_hasLost) {
					while (!m_inputQueue.empty()) m_inputQueue.pop();
					m_snake.m_direction = prev;
				} else step();
			}
		}
	}
}

#define DIRECTION_BUTTON(_direction)                                                          \
	if (ImGui::ArrowButton(directionToString(_direction).c_str(), ImGuiDir_##_direction)) {   \
		m_snake.m_direction = _direction;                                                     \
		step();                                                                               \
		checkDeath();                                                                         \
	}
IMGUI_ONLY(void Game::debugGUI() {
	ImGui::Begin("Debug Window");
	std::string currentDirection = directionToString(m_snake.m_direction);
	ImGui::Text("Current direction: %s", currentDirection.c_str());
	ImGui::Text("Step count: %i", m_stepCount);
	ImGui::Text("Time since step: %f", m_timeSinceStep);
	if (m_isSaveDirty) ImGui::Text("Save is dirty.");
	if (m_restartButtonHeld) ImGui::Text("Restart button is held.");
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Checkbox("Pause game", &m_isPaused);
	ImGui::Text("Master Volume");
	ImGui::SliderFloat("##master-volume", &m_masterVolume, 0.f, 5.f);
	if (ImGui::Button("Step")) {
		step();
		checkDeath();
	}
	ImGui::SameLine();
	if (ImGui::Button("Expand")) {
		m_snake.m_tiles.push_back(m_snake.getNextTile());
		checkDeath();
	}
	ImGui::NewLine();
	ImGui::Indent(20.f);
	ImGui::Indent(30.f);
	DIRECTION_BUTTON(Up);
	ImGui::Unindent(30.f);
	DIRECTION_BUTTON(Left);
	ImGui::SameLine(0.f, 36.f);
	DIRECTION_BUTTON(Right);
	ImGui::Indent(30.f);
	DIRECTION_BUTTON(Down);
	ImGui::Unindent(30.f);
	ImGui::Unindent(20.f);
	ImGui::NewLine();
	if (ImGui::Button("+")) {
		advanceScore();
		m_isSaveDirty = true;
	}
	ImGui::SameLine();
	ImGui::Text("Advance score (disables saving to file)");
	ImGui::Text("Start length");
	size_t min = 1; size_t max = 15;
	ImGui::SliderScalar("##start-length", ImGuiDataType_U64, &m_snake.m_startLength, &min, &max, "%d");
	ImGui::Text("Start speed");
	ImGui::SliderFloat("##start-speed", &m_startSpeed, 0, 50);
	ImGui::Text("Current speed");
	ImGui::SliderFloat("##current-speed", &m_speed, 0, 50);
	ImGui::End();
})

void Game::handleRestartButton(float resizeRatio) {
	static Texture2D const restartBtn = [&] {
		Image image = GenImageColor(2 * RESTART_BUTTON_SIZE, 2 * RESTART_BUTTON_SIZE, BLANK);
		ImageDrawCircleV(&image, { RESTART_BUTTON_SIZE, RESTART_BUTTON_SIZE }, RESTART_BUTTON_SIZE, RESTART_BUTTON_OUTER_COLOR);
		ImageDrawCircleV(&image, { RESTART_BUTTON_SIZE, RESTART_BUTTON_SIZE }, RESTART_BUTTON_SIZE * .65f, RESTART_BUTTON_INNER_COLOR);
		ImageDrawCircleV(&image, { RESTART_BUTTON_SIZE, RESTART_BUTTON_SIZE }, RESTART_BUTTON_SIZE * .4f, RESTART_BUTTON_OUTER_COLOR);
		ImageDrawTriangle(&image,
			{ RESTART_BUTTON_SIZE * .15f, RESTART_BUTTON_SIZE / 2 },
			{ RESTART_BUTTON_SIZE * .15f, RESTART_BUTTON_SIZE * 1.5f },
			{ RESTART_BUTTON_SIZE, RESTART_BUTTON_SIZE },
		RESTART_BUTTON_OUTER_COLOR);
		ImageDrawTriangle(&image,
			{ RESTART_BUTTON_SIZE * .38f, RESTART_BUTTON_SIZE * .57f },
			{ RESTART_BUTTON_SIZE * .4f,  RESTART_BUTTON_SIZE * .92f },
			{ RESTART_BUTTON_SIZE * .75f, RESTART_BUTTON_SIZE * .89f },
		RESTART_BUTTON_INNER_COLOR);
		Texture2D ret = LoadTextureFromImage(image);
		UnloadImage(image);
		return ret;
	}();
	if (m_hasLost) {
		DrawTextureV(restartBtn, RESTART_BUTTON_INFO.origin, WHITE);
		float gameSize = std::min(m_screenSize.x, m_screenSize.y);
		if (CheckCollisionPointCircle(
			// needed because otherwise it checks for clicks in the unresized original position
			GetMousePosition(), {
				RESTART_BUTTON_INFO.center.x * resizeRatio + (m_screenSize.x - gameSize) / 2,
				RESTART_BUTTON_INFO.center.y * resizeRatio + (m_screenSize.y - gameSize) / 2
			// here too
			}, RESTART_BUTTON_INFO.radius * resizeRatio
		)) {
			DrawCircleLinesV(RESTART_BUTTON_INFO.center, RESTART_BUTTON_INFO.radius - 1.5f, RAYWHITE);
			if (IsMouseButtonDown(0)) {
				m_restartButtonHeld = true;
				DrawCircleV(RESTART_BUTTON_INFO.center, RESTART_BUTTON_INFO.radius, {.a = 70});
			} else if (m_restartButtonHeld) m_shouldRestart = true;
		} else if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER)) {
			m_restartButtonHeld = true;
			DrawCircleV(RESTART_BUTTON_INFO.center, RESTART_BUTTON_INFO.radius, {.a = 70});
		} else if ((IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_ENTER)) && m_restartButtonHeld) m_shouldRestart = true;
		else {
			m_restartButtonHeld = false;
			DrawTextureV(restartBtn, RESTART_BUTTON_INFO.origin, WHITE);
		}
	}
}

void Game::draw() const {
	ClearBackground({ 40, 140, 50, 255 });

	static Color constexpr MAIN_COLOR = { 160, 255, 96, 255 };
	static Color constexpr ALT_COLOR = { 150, 235, 85, 255 };
	{
		static Texture2D const bg = [&] {
			RenderTexture2D target = LoadRenderTexture(DEFAULT_SCREEN_SIZE.x, DEFAULT_SCREEN_SIZE.y);
			BeginTextureMode(target);
			bool alternateColor = false;
			for (std::array<Rectangle, GRID_SIZE> const& line : GRID) {
				for (Rectangle const& rect : line) {
					DrawRectangleRec(rect, alternateColor ? ALT_COLOR : MAIN_COLOR);
					alternateColor = !alternateColor;
				}
			}
			EndTextureMode();
			return target.texture;
		}();
		DrawTextureV(bg, { }, WHITE);
	}

	{
		for (Apple const& applePos : m_apples) {
			Rectangle const& appleRec = recFromIndices(applePos, GRID);
			DrawTextureEx(m_resourceManager.getTexture("apple"),
				{ appleRec.x + TILE_EDGE_SIZE, appleRec.y + TILE_EDGE_SIZE }, 0.f,
				(FREE_SPACE - 20.f) / USED_TILE_SPACE, WHITE
			);
		}
	}

	m_snake.draw();

	if (m_hasLost) {
		static Texture2D const loseText = [&] {
			Image image = ImageTextEx(GetFontDefault(),
				"You lost!", 125.f, 1.f, BLACK
			);
			Texture2D ret = LoadTextureFromImage(image);
			UnloadImage(image);
			return ret;
		}();
		DrawTextureV(loseText, {
			(DEFAULT_SCREEN_SIZE.x - loseText.width) / 2,
			(DEFAULT_SCREEN_SIZE.y - loseText.height) / 2 - 50.f,
		}, WHITE);
	}

	drawOverlay();
}

void Game::drawOverlay() const {
	{
		Texture2D const apple = m_resourceManager.getTexture("apple");
		DrawTextureV(apple, {
			DEFAULT_SCREEN_SIZE.x - 100.f,
			(FREE_SPACE - apple.height) / 2,
		}, WHITE);
		DrawTextEx(GetFontDefault(),
			TextFormat("%i", m_score),
			{ DEFAULT_SCREEN_SIZE.x - 60.f, 15.f }, 25.f, 2.5f, BLACK
		);
		Texture2D const trophy = m_resourceManager.getTexture("trophy");
		DrawTextureV(trophy, {
			DEFAULT_SCREEN_SIZE.x - 200.f,
			(FREE_SPACE - trophy.height) / 2,
		}, WHITE);
		DrawTextEx(GetFontDefault(),
			TextFormat("%i", m_saveData.highScore),
			{ DEFAULT_SCREEN_SIZE.x - 160.f, 15.f },
			25.f, 2.5f, BLACK
		);
	}
	{
		static float constexpr ARROW_SIZE = FREE_SPACE - 20.f;
		static Texture2D const arrow = [&] {
			Image image = GenImageColor(ARROW_SIZE, ARROW_SIZE, BLANK);
			ImageDrawTriangle(&image,
				{ 0.f, ARROW_SIZE / 2 },
				{ ARROW_SIZE / 2, 0.f },
				{ ARROW_SIZE, ARROW_SIZE / 2 },
			BLACK);
			ImageDrawRectangleRec(&image, { 7.5f, ARROW_SIZE / 2, ARROW_SIZE / 2, ARROW_SIZE / 2 }, BLACK);
			Texture2D ret = LoadTextureFromImage(image);
			UnloadImage(image);
			return ret;
		}();
		DrawTextureEx(arrow,
			{ DEFAULT_SCREEN_SIZE.x / 2 - ARROW_SIZE / 2, 10.f },
			0.f,   1.f,
			(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && !m_hasLost ? BLACK : Color{.a = 70}
		);
		DrawTextureEx(arrow,
			{ DEFAULT_SCREEN_SIZE.x / 2 + ARROW_SIZE / 2, DEFAULT_SCREEN_SIZE.y - 10.f },
			180.f, 1.f,
			(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  && !m_hasLost ? BLACK : Color{.a = 70}
		);
		DrawTextureEx(arrow,
			{ 10.f, DEFAULT_SCREEN_SIZE.y / 2 + ARROW_SIZE / 2 },
			-90.f, 1.f,
			(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  && !m_hasLost ? BLACK : Color{.a = 70}
		);
		DrawTextureEx(arrow,
			{ DEFAULT_SCREEN_SIZE.x - 10.f, DEFAULT_SCREEN_SIZE.y / 2 - ARROW_SIZE / 2 },
			90.f,  1.f,
			(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && !m_hasLost ? BLACK : Color{.a = 70}
		);
	}
	{
		static Color constexpr COLOR = { 32, 32, 32, 255 };
		static Texture2D const speaker = [&] {
			RenderTexture2D rt = LoadRenderTexture(35, 50);
			BeginTextureMode(rt); {
				DrawRectangleRounded({ 0.f, 12.5f, 20.f, 25.f }, .3f, 5, COLOR);
				DrawTriangle(
					{ 10.f, 25.f },
					{ 35.f, 48.f },
					{ 35.f, 2.f },
				COLOR);
			} EndTextureMode();
			Image temp = LoadImageFromTexture(rt.texture);
			ImageFlipVertical(&temp);
			Texture2D ret = LoadTextureFromImage(temp);
			UnloadRenderTexture(rt);
			UnloadImage(temp);
			return ret;
		}();
		static Texture2D const soundWaves = [&] {
			RenderTexture2D rt = LoadRenderTexture(25, 50);
			BeginTextureMode(rt); {
				DrawRing({ 0.f, 25.f }, 10.f, 15.f, -60.f, 60.f, 10, COLOR);
				DrawRing({ 0.f, 25.f }, 20.f, 25.f, -50.f, 50.f, 10, COLOR);
			} EndTextureMode();
			Image temp = LoadImageFromTexture(rt.texture);
			ImageFlipVertical(&temp);
			Texture2D ret = LoadTextureFromImage(temp);
			UnloadRenderTexture(rt);
			UnloadImage(temp);
			return ret;
		}();
		DrawTextureV(speaker, { FREE_SPACE + 45.f, 0.f }, WHITE);
		if (m_muted) DrawLineEx({ FREE_SPACE + 50.f, 5.f }, { FREE_SPACE + 90.f, 45.f }, 5.f, BLACK);
		else DrawTextureV(soundWaves, { FREE_SPACE + 78.f, 0.f }, WHITE);
		DrawTextEx(GetFontDefault(), "M", { FREE_SPACE + 5.f, 5.f }, 45.f, 1e20, BLACK);
	}
}
void Game::deinit() {
	m_resourceManager.deinit();
	UnloadRenderTexture(m_screen);
	CloseAudioDevice();
	IMGUI_ONLY(rlImGuiShutdown());
	CloseWindow();
DESKTOP_ONLY(
	if (m_shouldSaveLogs || m_hadWarning) saveLogs();
	clipboard_free(m_cb);
	if (!m_isSaveDirty) saveData(SAVE_FILE);
)
}
