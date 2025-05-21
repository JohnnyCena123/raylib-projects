#include <array>
#include <queue>
#include <random>
#include <string>

#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>

int constexpr GRID_SIZE = 21;
float constexpr BLOCK_SIZE = 40.f;
float constexpr FREE_SPACE = 50.f;
float constexpr SCREEN_SIZE = GRID_SIZE * BLOCK_SIZE + FREE_SPACE * 2;

struct Tile { Rectangle rect; bool filled; };

using Grid = std::array<std::array<Tile, GRID_SIZE>, GRID_SIZE>;

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
	using Tiles = std::queue<std::array<int, 2>>;
	Tiles tiles; 

	std::array<int, 2> step(Grid& grid, Apple& applePos);

	Direction direction; 
};


void draw(Grid const& grid, Snake const& snake, Apple const& applePos);
bool checkDeath(Grid const& grid, Snake const& snake);
void gameStep(Grid& grid, Snake& snake, Apple& applePos);
void lose();

static inline constexpr Tile& tileFromIndices(std::array<int, 2>& indices, Grid& grid) { return grid[indices[1]][indices[0]]; }

static inline Tile constexpr tileFromIndices(std::array<int, 2> const& indices, Grid const& grid) { return grid[indices[1]][indices[0]]; }

int main() {

	static std::array<int, 2> constexpr START_POS = { 12, 10, };
	
	Grid grid{};
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			grid[i][j] = { Rectangle{
				FREE_SPACE + j * BLOCK_SIZE,
				FREE_SPACE + i * BLOCK_SIZE,
				BLOCK_SIZE, BLOCK_SIZE,
			}, false, };
		}		
	}

	Apple apple = START_POS;

	Snake snake = { { }, Left, };
	snake.tiles.push(START_POS);

	gameStep(grid, snake, apple); apple = { snake.tiles.back()[0] - 1, snake.tiles.back()[1], };
	gameStep(grid, snake, apple); apple = { snake.tiles.back()[0] - 1, snake.tiles.back()[1], };
	
	InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Snake");

	rlImGuiSetup(true);

	SetTargetFPS(60);

	int stepCount = 0;

	bool paused;

	auto checkDeathWrapper = [&](bool step) {
		if (step) gameStep(grid, snake, apple);
		if (checkDeath(grid, snake)) {
			lose();
			paused = true;
		}
	};

	while (!WindowShouldClose()) {
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( { 160, 255, 96, 255, } );

		if (IsKeyPressed(KEY_UP))    { snake.direction = Up;    checkDeathWrapper(false); }
		if (IsKeyPressed(KEY_DOWN))  { snake.direction = Down;  checkDeathWrapper(false); }
		if (IsKeyPressed(KEY_LEFT))  { snake.direction = Left;  checkDeathWrapper(false); }
		if (IsKeyPressed(KEY_RIGHT)) { snake.direction = Right; checkDeathWrapper(false); }

		ImGui::Begin("debug");

		ImGui::Checkbox("Pause game", &paused);

		auto currentDirection = directionToString(snake.direction);
		ImGui::Text("Current direction: %s", currentDirection.c_str());

		if (ImGui::Button("Step"))   checkDeathWrapper(true);
		ImGui::SameLine();
		if (ImGui::Button("Expand")) {
			apple = snake.tiles.back();
			checkDeathWrapper(true);
		}

		#define DIRECTION_BUTTON(direction_)                                                      \
		if (ImGui::ArrowButton(directionToString(direction_).c_str(), ImGuiDir_##direction_)) {   \
			snake.direction = direction_;                                                         \
			checkDeathWrapper(true);                                                                          \
		}                                                                                         \
		ImGui::SameLine()
		
		DIRECTION_BUTTON(Up);
		DIRECTION_BUTTON(Down);
		DIRECTION_BUTTON(Left);
		DIRECTION_BUTTON(Right);

		ImGui::NewLine();

		int i = 0;
		for (auto& line : grid) {
			for (auto& [_, filled] : line) {
				ImGui::Checkbox((std::string("##") + std::to_string(i)).c_str(), &filled);
				ImGui::SameLine();
				i++;
			}
			ImGui::NewLine();
		}
		
		ImGui::End();

		if (GetTime() * 3 > stepCount) {
			stepCount++;

			if (!paused) checkDeathWrapper(true);
		}

		draw(grid, snake, apple);

		rlImGuiEnd();
		EndDrawing();
	}

	rlImGuiShutdown();
	CloseWindow();

	return 0;
}



void draw(Grid const& grid, Snake const& snake, Apple const& applePos) {
	
	static auto apple = [] {
		auto image = LoadImage("resources/apple.png");

		ImageResize(&image, BLOCK_SIZE - 10.f, BLOCK_SIZE - 10.f);

		return LoadTextureFromImage(image);
	}();

	auto appleRect = tileFromIndices(applePos, grid).rect;
	DrawTextureV(apple, { appleRect.x + 5.f, appleRect.y + 5.f, }, WHITE);

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
		FREE_SPACE, GRID_SIZE * BLOCK_SIZE,
	}, BLUE);
	DrawRectangleRec({
		SCREEN_SIZE - FREE_SPACE, FREE_SPACE,
		FREE_SPACE, GRID_SIZE * BLOCK_SIZE,
	}, BLUE);

	auto copy = snake.tiles;
	while (!copy.empty()) {
		auto tile = copy.front();
		copy.pop();
		auto const& [rect, _] = tileFromIndices(tile, grid); 
		DrawRectangleRec(rect, tile == snake.tiles.back() ? PURPLE : RED);
	}

	for (auto const& line : grid) 
		for (auto const& [rect, _] : line)
			DrawRectangleLinesEx(rect, 1.f, SKYBLUE);

}

std::array<int, 2> Snake::step(Grid& grid, Apple& applePos) {

	std::random_device rd;
	std::mt19937 e{rd()};
	std::uniform_int_distribution<int> dist{0, GRID_SIZE - 1};
	
	std::array<int, 2> head = tiles.back();
	switch (direction) {
		case Up:    head[1]--; break;
		case Down:  head[1]++; break;
		case Left:  head[0]--; break;
		case Right: head[0]++; break;
	}
	if (head[0] < 0)          head[0] = head[0] + GRID_SIZE;
	if (head[0] >= GRID_SIZE) head[0] = head[0] - GRID_SIZE;
	if (head[1] < 0)          head[1] = head[1] + GRID_SIZE;
	if (head[1] >= GRID_SIZE) head[1] = head[1] - GRID_SIZE;
	tiles.push(head);

	tileFromIndices(head, grid).filled = true;

	auto back = tiles.front();

	if (CheckCollisionRecs(
		tileFromIndices(tiles.back(), grid).rect, 
		tileFromIndices(applePos, grid).rect)
	) applePos = { dist(e), dist(e), };
	else {
		tiles.pop();
		tileFromIndices(back, grid).filled = false;
	}

	return back;
}
void gameStep(Grid& grid, Snake& snake, Apple& applePos) {
	auto back = snake.step(grid, applePos);
}

bool checkDeath(Grid const& grid, Snake const& snake) {
	bool lose = false;

	auto head = snake.tiles.back();
	auto copy = snake.tiles;
	int tilesChecked = 0;
	while (!copy.empty()) {
		tilesChecked++;

		auto tile = copy.front();
		copy.pop();
		auto [rect, _] = tileFromIndices(tile, grid); 

		// to make it return true for the collision when its not on the same tile but the head is gonna touch the rect on thenext step
		// 1e-4 is used as the epsilon because FLT_EPSILON doesnt work lmao
		switch (snake.direction) {
			case Up:    rect.height += 1e-4; break;
			case Down:  rect.y      -= 1e-4; break;
			case Left:  rect.width  += 1e-4; break;
			case Right: rect.x      -= 1e-4; break;
		}

		if (tilesChecked < snake.tiles.size() && CheckCollisionRecs(tileFromIndices(head, grid).rect, rect)) lose = true;
	}

	return lose;
}

void lose() {

}