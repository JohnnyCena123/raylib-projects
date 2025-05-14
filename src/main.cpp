#include <array>
#include <queue>
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
void draw(Grid const& grid);

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
struct Snake { 
	using Tiles = std::queue<std::array<size_t, 2>>;
	Tiles tiles; 

	std::array<size_t, 2> step(Grid& grid, bool expand);

	Direction direction; 
};
void gameStep(Grid& grid, Snake& snake, bool expand);

int main(void) {
	
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

	Snake snake = { { }, Left, };
	snake.tiles.push({ 12, 10, });
	snake.step(grid, true);
	snake.step(grid, true);
	
	gameStep(grid, snake, false);

	InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Snake");
	
	rlImGuiSetup(true);

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( {160, 255, 96, 255, } );

		draw(grid);

		ImGui::Begin("debug");

		auto currentDirection = directionToString(snake.direction);
		ImGui::Text("Current direction: %s", currentDirection.c_str());

		if (ImGui::Button("Step"))   gameStep(grid, snake, false);
		ImGui::SameLine();
		if (ImGui::Button("Expand")) gameStep(grid, snake, true);

		#define DIRECTION_BUTTON(direction_) if (ImGui::ArrowButton(directionToString(direction_).c_str(), ImGuiDir_##direction_)) { \
			snake.direction = direction_;                                                                                          \
			gameStep(grid, snake, false);                                                                                         \
		}                                                                                                                         \
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

		rlImGuiEnd();
		EndDrawing();
	}

	rlImGuiShutdown();
	CloseWindow();

	return 0;
}

void draw(Grid const& grid) {
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

	for (auto const& line : grid) {
		for (auto const& [rect, filled] : line) {
			if (filled) {
				DrawRectangleRec(rect, RED);
			}
			DrawRectangleLinesEx(rect, 1.f, SKYBLUE);
		}
	}
}

std::array<size_t, 2> Snake::step(Grid& grid, bool expand) {
	auto back = tiles.front();

	if (!expand) tiles.pop();
	std::array<size_t, 2> head = tiles.back();
	switch (direction) {
		case Up:    head[1]--; break;
		case Down:  head[1]++; break;
		case Left:  head[0]--; break;
		case Right: head[0]++; break;
	}
	tiles.push(head);

	grid[head[1]][head[0]].filled = true;

	return back;
}

void gameStep(Grid& grid, Snake& snake, bool expand) {
	auto back = snake.step(grid, expand);
	if (!expand) grid[back[1]][back[0]].filled = false;
}