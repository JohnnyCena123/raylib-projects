#if defined(_WIN32) && defined(CMAKE_RELEASE_BUILD)
// holy shit i hate raylib + windows
// it compiles so shut up
#define _WINGDI_
#define _WINUSER_
#define _APISETCONSOLEL3_
#define _WINSPOOL_
#define _OLE2_H_
#define _INC_COMMDLG
#define _IMM_SDK_DEFINED_
#define _PLAYSOUNDAPI_H_
#include <windows.h>
int main();
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return main(); }
#endif

#include <algorithm>
#include <array>
#include <optional>
#include <deque>
#include <queue>
#include <random>
#include <string>

#include <raylib.h>
#include <rlgl.h>
#ifndef CMAKE_RELEASE_BUILD
	#include <imgui.h>
	#include <rlImGui.h>
	#define DEBUG_ONLY(...) __VA_ARGS__
#else
	#define DEBUG_ONLY(...)
#endif

int constexpr GRID_SIZE = 21;
float constexpr TILE_SIZE = 40.f;
float constexpr FREE_SPACE = 50.f;
float constexpr SCREEN_SIZE = GRID_SIZE * TILE_SIZE + FREE_SPACE * 2;

using Grid = std::array<std::array<Rectangle, GRID_SIZE>, GRID_SIZE>;

using Direction = enum { Up, Down, Left, Right, };
inline std::string constexpr directionToString(Direction direction) {
	switch (direction) {
		case Up:    return "up";
		case Down:  return "down";
		case Left:  return "left";
		case Right: return "right";
	}
	return "";
}

using Apple = std::array<int, 2>;

struct Snake { 
	using Tiles = std::deque<std::array<int, 2>>;
	Tiles tiles; 

	std::array<int, 2> getNextTile() const;
	void step(Grid& grid, Apple& applePos);

	Direction direction; 

	int score;
	static int s_maxScore;
};

int Snake::s_maxScore = 0;

void draw(Grid const& grid, Snake const& snake, Apple const& applePos, bool hasLost);

static inline Rectangle       constexpr& tileFromIndices(std::array<int, 2> const& indices, Grid&       grid) { return grid[indices[1]][indices[0]]; }
static inline Rectangle const constexpr& tileFromIndices(std::array<int, 2> const& indices, Grid const& grid) { return grid[indices[1]][indices[0]]; }

bool startGame();

int main() {
	
	InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Snake");

	DEBUG_ONLY(rlImGuiSetup(true));

	SetTargetFPS(60);

	while (startGame()) continue;

	DEBUG_ONLY(rlImGuiShutdown());
	CloseWindow();

	return 0;
}

bool startGame() {
	bool restart = false;

	static std::array<int, 2> constexpr START_POS = { 12, 10, };

	static int constexpr STEPS_PER_SECOND = 15;
	
	Grid grid{};
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			tileFromIndices({ j, i, }, grid) = {
				FREE_SPACE + j * TILE_SIZE,
				FREE_SPACE + i * TILE_SIZE,
				TILE_SIZE, TILE_SIZE,
			};
		}		
	}

	Apple apple = { START_POS[0] - 1, START_POS[1], };

	Snake snake = { { }, Left, -1, };
	snake.tiles.push_back(START_POS);
	snake.step(grid, apple);

	static int constexpr START_LENGTH = 3;
	for (int i = 2; i < START_LENGTH; i++) snake.tiles.push_back(snake.getNextTile());
	
	bool paused = false;
	bool hasLost = false;

	static auto checkDeath = [&]() { 
		if (snake.tiles.size() < 5) return; // no more of this
		int n = 0;
		for (auto const& tile : snake.tiles) if (tile == snake.getNextTile() && tile != snake.tiles.front()) {
			hasLost = true; 
			break;
		}
	};

	std::queue<Direction> inputQueue{};
	static auto processInput = [&](Direction direction) {
		if (inputQueue.size() > 5) return;
		bool changeDir = false;
		auto checkDir = inputQueue.empty() ? snake.direction : inputQueue.front();
		switch (direction) {
			case Up:    if (checkDir != Down  && checkDir != Up   ) changeDir = true; break;
			case Down:  if (checkDir != Up    && checkDir != Down ) changeDir = true; break;
			case Left:  if (checkDir != Right && checkDir != Left ) changeDir = true; break;
			case Right: if (checkDir != Left  && checkDir != Right) changeDir = true; break;
		}
		if (changeDir) inputQueue.push(direction);
	};

	double lastStepTime;
	while (!WindowShouldClose()) {
		BeginDrawing();
		DEBUG_ONLY(rlImGuiBegin());

		ClearBackground( { 160, 255, 96, 255, } );

		draw(grid, snake, apple, hasLost);

		{
			if (!hasLost) {
				if      (IsKeyPressed(KEY_UP))    processInput(Up);
				else if (IsKeyPressed(KEY_DOWN))  processInput(Down);
				else if (IsKeyPressed(KEY_LEFT))  processInput(Left);
				else if (IsKeyPressed(KEY_RIGHT)) processInput(Right);
			}
			if ((GetTime() - lastStepTime) * STEPS_PER_SECOND > 1) {
				lastStepTime = GetTime();
				if (!paused && !hasLost) {
					Direction prev = snake.direction;
					if (!inputQueue.empty()) {
						snake.direction = inputQueue.front();
						inputQueue.pop();
					}
					checkDeath();
					if (hasLost) {
						while (!inputQueue.empty()) inputQueue.pop();
						snake.direction = prev;
					} else snake.step(grid, apple);
				}
			}
		}

		DEBUG_ONLY(
			ImGui::Begin("Debug Window");

			ImGui::Checkbox("Pause game", &paused);

			auto currentDirection = directionToString(snake.direction);
			ImGui::Text("Current direction: %s", currentDirection.c_str());

			if (ImGui::Button("Step")) {
				snake.step(grid, apple);
				checkDeath();
			}
			ImGui::SameLine();
			if (ImGui::Button("Expand")) {
				snake.tiles.push_back(snake.getNextTile());
				checkDeath();
			}

		#define DIRECTION_BUTTON(direction_)                                                          \
			if (ImGui::ArrowButton(directionToString(direction_).c_str(), ImGuiDir_##direction_)) {   \
				processInput(direction_);                                                             \
				snake.step(grid, apple);                                                              \
				checkDeath();                                                                         \
			}                                                                                         \
			ImGui::SameLine()
			
			DIRECTION_BUTTON(Up);
			DIRECTION_BUTTON(Down);
			DIRECTION_BUTTON(Left);
			DIRECTION_BUTTON(Right);

			ImGui::NewLine();
			
			ImGui::End();
		)

		{
			static float constexpr RESTART_BTN_RADIUS = 75.f;
			static auto restartBtn = []() {
				static Color constexpr OUTER_COLOR = { 0, 208, 51, 255 };
				static Color constexpr INNER_COLOR = { 220, 220, 220, 255, };

				auto image = GenImageColor(2 * RESTART_BTN_RADIUS, 2 * RESTART_BTN_RADIUS, BLANK);
				ImageDrawCircleV(&image, { RESTART_BTN_RADIUS, RESTART_BTN_RADIUS, }, RESTART_BTN_RADIUS, OUTER_COLOR);
				ImageDrawCircleV(&image, { RESTART_BTN_RADIUS, RESTART_BTN_RADIUS, }, RESTART_BTN_RADIUS * .65f, INNER_COLOR);
				ImageDrawCircleV(&image, { RESTART_BTN_RADIUS, RESTART_BTN_RADIUS, }, RESTART_BTN_RADIUS * .4f, OUTER_COLOR);
				ImageDrawTriangle(&image, 
					{ RESTART_BTN_RADIUS * .15f, RESTART_BTN_RADIUS / 2, }, 
					{ RESTART_BTN_RADIUS * .15f, RESTART_BTN_RADIUS * 1.5f, }, 
					{ RESTART_BTN_RADIUS, RESTART_BTN_RADIUS, }, 
				OUTER_COLOR);
				ImageDrawTriangle(&image, 
					{ RESTART_BTN_RADIUS * .38f, RESTART_BTN_RADIUS * .57f }, 
					{ RESTART_BTN_RADIUS * .4f,  RESTART_BTN_RADIUS * .92f, }, 
					{ RESTART_BTN_RADIUS * .75f, RESTART_BTN_RADIUS * .89f, }, 
				INNER_COLOR);

				return LoadTextureFromImage(image);
			}();
			static struct {
				Vector2 origin;
				Vector2 center;
				float radius;
			} constexpr restartBtnCircle = { {
					SCREEN_SIZE / 2 - RESTART_BTN_RADIUS, 
					SCREEN_SIZE / 2, 
				}, {
					SCREEN_SIZE / 2, 
					SCREEN_SIZE / 2 + RESTART_BTN_RADIUS, 
				}, RESTART_BTN_RADIUS,
			};
			if (hasLost) {
				DrawTextureV(restartBtn, restartBtnCircle.origin, WHITE);
				if (CheckCollisionPointCircle(
					GetMousePosition(), restartBtnCircle.center,
					restartBtnCircle.radius
				)) {
					DrawCircleLinesV(restartBtnCircle.center, restartBtnCircle.radius, RAYWHITE);
					if (IsMouseButtonDown(0)) DrawCircleV(
						restartBtnCircle.center, restartBtnCircle.radius, {.a = 70}
					);
					else if (IsMouseButtonReleased(0)) restart = true;
				}
			}
		}

		DEBUG_ONLY(rlImGuiEnd());
		EndDrawing();
		if (restart) return true;
	}

	return false;
}

void draw(Grid const& grid, Snake const& snake, Apple const& applePos, bool hasLost) {
	static float constexpr EMPTY_TILE_SPACE = 5.f; // on each side

	{
		for (auto const& line : grid) 
			for (auto const& rect : line)
				DrawRectangleLinesEx(rect, 1.f, SKYBLUE);
	}
	{
		static auto apple = [] {
			auto image = LoadImage("resources/apple.png");
			ImageResize(&image, TILE_SIZE - EMPTY_TILE_SPACE * 2, TILE_SIZE - EMPTY_TILE_SPACE * 2);
			return LoadTextureFromImage(image);
		}();
		auto appleRect = tileFromIndices(applePos, grid);
		DrawTextureV(apple, { appleRect.x + EMPTY_TILE_SPACE, appleRect.y + EMPTY_TILE_SPACE, }, WHITE);
	}
	{
		DrawRectangleRec({
			0.f, 0.f,
			SCREEN_SIZE, FREE_SPACE,
		}, BLUE);
		DrawRectangleRec({
			0.f, SCREEN_SIZE - FREE_SPACE,
			SCREEN_SIZE, FREE_SPACE,
		}, BLUE);

		DrawRectangleRec({
			0.f, FREE_SPACE,
			FREE_SPACE, GRID_SIZE * TILE_SIZE,
		}, BLUE);
		DrawRectangleRec({
			SCREEN_SIZE - FREE_SPACE, FREE_SPACE,
			FREE_SPACE, GRID_SIZE * TILE_SIZE,
		}, BLUE);
	}
	{
		auto head = snake.tiles.back();
		std::optional<std::array<int, 2>> lastDrawn = std::nullopt;
		for (auto const& tile : snake.tiles) {
			auto const& rect = tileFromIndices(tile, grid); 
			bool didTeleport = false;
			auto drawRect = Rectangle{ 
				rect.x + EMPTY_TILE_SPACE, rect.y + EMPTY_TILE_SPACE, 
				rect.width - EMPTY_TILE_SPACE * 2, rect.height - EMPTY_TILE_SPACE * 2, 
			}; //  just a square in the middle of the tile unless told otherwise
			{
				if (lastDrawn) { // first tile, doesnt connect to anything
					auto diff = std::array{ tile[0] - (*lastDrawn)[0], tile[1] - (*lastDrawn)[1], };
					if (abs(diff[0]) > 1 || abs(diff[1]) > 1) didTeleport = true;
					else { // it should try to connect to the previous tile
						switch (diff[0]) {
							case -1: drawRect = { 
								rect.x + EMPTY_TILE_SPACE, rect.y + EMPTY_TILE_SPACE, 
								rect.width, rect.height - EMPTY_TILE_SPACE * 2, 
							}; break;
							case 1: drawRect = { 
								rect.x - EMPTY_TILE_SPACE, rect.y + EMPTY_TILE_SPACE, 
								rect.width, rect.height - EMPTY_TILE_SPACE * 2, 
							}; break;
							case 0: switch (diff[1]) {
								case -1: drawRect = { 
									rect.x + EMPTY_TILE_SPACE, rect.y + EMPTY_TILE_SPACE, 
									rect.width - EMPTY_TILE_SPACE * 2, rect.height, 
								}; break;
								case 1: drawRect = { 
									rect.x + EMPTY_TILE_SPACE, rect.y - EMPTY_TILE_SPACE, 
									rect.width - EMPTY_TILE_SPACE * 2, rect.height, 
								}; break;
							} break;
						}
					}
				}
				DrawRectangleRec(drawRect, RED);
			}
			{
				if (tile == head) {
					auto const& thirdToLast = snake.tiles.at(snake.tiles.size() - 3);
					auto gradientRect = drawRect;
					if (!didTeleport && // we dont want the gradient to go beyond the borders of the screen
						(head[0] == thirdToLast[0] || head[1] == thirdToLast[1]) // i tried to add a corner gradient to make it look better but it didnt work so
					) {
						switch (snake.direction) {
							case Up:    gradientRect.height += TILE_SIZE; break;
							case Down:  gradientRect.y -= TILE_SIZE; gradientRect.height += TILE_SIZE;; break;
							case Left:  gradientRect.width += TILE_SIZE; break;
							case Right: gradientRect.x -= TILE_SIZE; gradientRect.width += TILE_SIZE; break;
						}
					}
					switch (snake.direction) {
						case Up:    DrawRectangleGradientEx(gradientRect, PURPLE, BLANK, BLANK, PURPLE); break;
						case Down:  DrawRectangleGradientEx(gradientRect, BLANK, PURPLE, PURPLE, BLANK); break;
						case Left:  DrawRectangleGradientEx(gradientRect, PURPLE, PURPLE, BLANK, BLANK); break;
						case Right: DrawRectangleGradientEx(gradientRect, BLANK, BLANK, PURPLE, PURPLE); break;
					}
				}
			}
			
			lastDrawn = tile;
		}
	}
	{
		DrawTextEx(GetFontDefault(),
			(std::string("Score: ") + std::to_string(snake.score)).c_str(),
			{ FREE_SPACE + 50.f, 15.f, }, 25.f, 2.5f, BLACK
		);

		auto text = std::string("Max Score: ") + std::to_string(Snake::s_maxScore);
		DrawTextEx(GetFontDefault(),
			text.c_str(), 
			{ SCREEN_SIZE - FREE_SPACE - MeasureTextEx(GetFontDefault(), 
				text.c_str(), 25.f, 2.5f).x - 50.f, 
				15.f, 
			}, 
			25.f, 2.5f, BLACK
		);
	}
	{
		static auto loseText = []() {
			auto image = ImageTextEx(GetFontDefault(),
				"You lost!", 125.f, 1.f, BLACK
			);
			return LoadTextureFromImage(image);
		}();
		if (hasLost) DrawTextureV(loseText, { 
			(SCREEN_SIZE - loseText.width) / 2, 
			(SCREEN_SIZE - loseText.height) / 2 - 50.f, 
		}, WHITE);
	}
	{
		static float constexpr ARROW_SIZE = FREE_SPACE - 20.f;
		static auto arrow = []() {
			auto image = GenImageColor(ARROW_SIZE, ARROW_SIZE, BLANK);
			ImageDrawTriangle(&image, 
				{ 0.f, ARROW_SIZE / 2, }, 
				{ ARROW_SIZE / 2, 0.f, }, 
				{ ARROW_SIZE, ARROW_SIZE / 2, }, 
			BLACK);
			ImageDrawRectangleRec(&image, { 7.5f, ARROW_SIZE / 2, ARROW_SIZE / 2, ARROW_SIZE / 2, }, BLACK);

			return LoadTextureFromImage(image);
		}();

		DrawTextureEx(arrow, { SCREEN_SIZE / 2 - ARROW_SIZE / 2, 10.f, },               0.f,   1.f, IsKeyDown(KEY_UP)    && !hasLost ? BLACK : Color{.a = 70});
		DrawTextureEx(arrow, { SCREEN_SIZE / 2 + ARROW_SIZE / 2, SCREEN_SIZE - 10.f, }, 180.f, 1.f, IsKeyDown(KEY_DOWN)  && !hasLost ? BLACK : Color{.a = 70});
		DrawTextureEx(arrow, { 10.f, SCREEN_SIZE / 2 + ARROW_SIZE / 2, },               -90.f, 1.f, IsKeyDown(KEY_LEFT)  && !hasLost ? BLACK : Color{.a = 70});
		DrawTextureEx(arrow, { SCREEN_SIZE - 10.f, SCREEN_SIZE / 2 - ARROW_SIZE / 2, }, 90.f,  1.f, IsKeyDown(KEY_RIGHT) && !hasLost ? BLACK : Color{.a = 70});
	}
}
	
std::array<int, 2> Snake::getNextTile() const {
	auto ret = tiles.back();
	switch (direction) {
		case Up:    ret[1]--; break;
		case Down:  ret[1]++; break;
		case Left:  ret[0]--; break;
		case Right: ret[0]++; break;
	}
	if (ret[0] < 0)          ret[0] = ret[0] + GRID_SIZE;
	if (ret[0] >= GRID_SIZE) ret[0] = ret[0] - GRID_SIZE;
	if (ret[1] < 0)          ret[1] = ret[1] + GRID_SIZE;
	if (ret[1] >= GRID_SIZE) ret[1] = ret[1] - GRID_SIZE;

	return ret;	
}
void Snake::step(Grid& grid, Apple& applePos) {

	static std::random_device rd;
	static std::mt19937 e{rd()};
	static std::uniform_int_distribution<int> dist{0, GRID_SIZE - 1};
	
	tiles.push_back(getNextTile());

	auto back = tiles.front();
	if (CheckCollisionRecs(
		tileFromIndices(tiles.back(), grid), 
		tileFromIndices(applePos, grid))
	) {
		while (std::find(tiles.begin(), tiles.end(), applePos) != tiles.end()) 
			applePos = { dist(e), dist(e), };
		score++;
		if (score > s_maxScore) s_maxScore++;
	} else tiles.pop_front();
}
