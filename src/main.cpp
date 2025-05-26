#include <array>
#include <optional>
#include <queue>
#include <random>
#include <string>

#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>

int constexpr GRID_SIZE = 21;
float constexpr TILE_SIZE = 25.f;
float constexpr FREE_SPACE = 50.f;
float constexpr SCREEN_SIZE = GRID_SIZE * TILE_SIZE + FREE_SPACE * 2;

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

	void step(Grid& grid, Apple& applePos);

	Direction direction; 

	int score = -2;
};


void draw(Grid const& grid, Snake const& snake, Apple const& applePos, bool hasLost);
bool checkDeath(Grid const& grid, Snake const& snake);
void gameStep(Grid& grid, Snake& snake, Apple& applePos);

static inline Tile constexpr& tileFromIndices(std::array<int, 2>&       indices, Grid&       grid) { return grid[indices[1]][indices[0]]; }
static inline Tile constexpr  tileFromIndices(std::array<int, 2> const& indices, Grid const& grid) { return grid[indices[1]][indices[0]]; }

int main() {

	static std::array<int, 2> constexpr START_POS = { 12, 10, };

	static int constexpr STEPS_PER_SECOND = 10;
	
	Grid grid{};
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			grid[i][j] = { Rectangle{
				FREE_SPACE + j * TILE_SIZE,
				FREE_SPACE + i * TILE_SIZE,
				TILE_SIZE, TILE_SIZE,
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

	bool paused = false;
	bool hasLost = false;

	auto checkDeathWrapper = [&]() { if (checkDeath(grid, snake)) hasLost = true; };
	auto stepWrapper = [&]() {
		gameStep(grid, snake, apple);
		checkDeathWrapper();
	};

	auto processInput = [&](Direction direction) {
		bool changed = false;
		switch (direction) {
			case Up:    if (snake.direction != Down  && snake.direction != Up   ) changed = true; break;
			case Down:  if (snake.direction != Up    && snake.direction != Down ) changed = true; break;
			case Left:  if (snake.direction != Right && snake.direction != Left ) changed = true; break;
			case Right: if (snake.direction != Left  && snake.direction != Right) changed = true; break;
		}
		if (changed) {
			snake.direction = direction;
			checkDeathWrapper();
		}
	};

	std::queue<Direction> inputQueue{};

	while (!WindowShouldClose()) {
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( { 160, 255, 96, 255, } );

		if (IsKeyPressed(KEY_UP))    { inputQueue.push(Up);    }
		if (IsKeyPressed(KEY_DOWN))  { inputQueue.push(Down);  }
		if (IsKeyPressed(KEY_LEFT))  { inputQueue.push(Left);  }
		if (IsKeyPressed(KEY_RIGHT)) { inputQueue.push(Right); }

		ImGui::Begin("debug");

		ImGui::Checkbox("Pause game", &paused);

		auto currentDirection = directionToString(snake.direction);
		ImGui::Text("Current direction: %s", currentDirection.c_str());

		if (ImGui::Button("Step")) stepWrapper();
		ImGui::SameLine();
		if (ImGui::Button("Expand")) {
			switch (snake.direction) {
				case Up:    apple = { snake.tiles.back()[0]    , snake.tiles.back()[1] - 1, }; break;
				case Down:  apple = { snake.tiles.back()[0]    , snake.tiles.back()[1] + 1, }; break;
				case Left:  apple = { snake.tiles.back()[0] - 1, snake.tiles.back()[1],     }; break;
				case Right: apple = { snake.tiles.back()[0] + 1, snake.tiles.back()[1],     }; break;
			}
			
			stepWrapper();
		}

	#define DIRECTION_BUTTON(direction_)                                                          \
		if (ImGui::ArrowButton(directionToString(direction_).c_str(), ImGuiDir_##direction_)) {   \
			processInput(direction_);                                                             \
			stepWrapper();                                                                        \
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

		if (GetTime() * STEPS_PER_SECOND > stepCount) {
			stepCount++;
			if (!paused && !hasLost) {
				if (!inputQueue.empty()) {
					processInput(inputQueue.front());
					inputQueue.pop();
				}
				stepWrapper();
			}
		}

		draw(grid, snake, apple, hasLost);

		rlImGuiEnd();
		EndDrawing();
	}

	rlImGuiShutdown();
	CloseWindow();

	return 0;
}


void draw(Grid const& grid, Snake const& snake, Apple const& applePos, bool hasLost) {
	static float constexpr EMPTY_SQUARE_SPACE = 5.f; // on each side
	
	static auto apple = [] {
		auto image = LoadImage("resources/apple.png");

		ImageResize(&image, TILE_SIZE - EMPTY_SQUARE_SPACE * 2, TILE_SIZE - EMPTY_SQUARE_SPACE * 2);

		return LoadTextureFromImage(image);
	}();

	for (auto const& line : grid) 
		for (auto const& [rect, _] : line)
			DrawRectangleLinesEx(rect, 1.f, SKYBLUE);

	auto appleRect = tileFromIndices(applePos, grid).rect;
	DrawTextureV(apple, { appleRect.x + EMPTY_SQUARE_SPACE, appleRect.y + EMPTY_SQUARE_SPACE, }, WHITE);

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

	auto head = snake.tiles.back();
	auto copy = snake.tiles;

	std::optional<std::array<int, 2>> lastDrawn;
	std::array<int, 2> thirdToLast;
	while (!copy.empty()) {
		auto tile = copy.front();
		auto const& [rect, _] = tileFromIndices(tile, grid); 
		Rectangle drawRect;
		auto diff = std::array{ tile[0] - (*lastDrawn)[0], tile[1] - (*lastDrawn)[1], };
		if (
			!lastDrawn ||                               // first tile, doesnt connect to anything
			abs(diff[0]) > 1 || abs(diff[1]) > 1   // if the snake teleported to the other side of the grid it shouldnt try to connect
		) drawRect = { 
			rect.x + EMPTY_SQUARE_SPACE, rect.y + EMPTY_SQUARE_SPACE, 
			rect.width - EMPTY_SQUARE_SPACE * 2, rect.height - EMPTY_SQUARE_SPACE * 2, 
		}; //  just a square in the middle of the tile
		else { // it should try to connect to the previous tile
			switch (diff[0]) {
				case -1: drawRect = { 
					rect.x + EMPTY_SQUARE_SPACE, rect.y + EMPTY_SQUARE_SPACE, 
					rect.width, rect.height - EMPTY_SQUARE_SPACE * 2, 
				}; break;
				case 1: drawRect = { 
					rect.x - EMPTY_SQUARE_SPACE, rect.y + EMPTY_SQUARE_SPACE, 
					rect.width, rect.height - EMPTY_SQUARE_SPACE * 2, 
				}; break;
				case 0: switch (diff[1]) {
					case -1: drawRect = { 
						rect.x + EMPTY_SQUARE_SPACE, rect.y + EMPTY_SQUARE_SPACE, 
						rect.width - EMPTY_SQUARE_SPACE * 2, rect.height, 
					}; break;
					case 1: drawRect = { 
						rect.x + EMPTY_SQUARE_SPACE, rect.y - EMPTY_SQUARE_SPACE, 
						rect.width - EMPTY_SQUARE_SPACE * 2, rect.height, 
					}; break;
				} break;
			}
		}
		DrawRectangleRec(drawRect, RED);

		drawRect = { 
			rect.x + EMPTY_SQUARE_SPACE, rect.y + EMPTY_SQUARE_SPACE, 
			rect.width - EMPTY_SQUARE_SPACE * 2, rect.height - EMPTY_SQUARE_SPACE * 2, 
		};
		if (tile == head) {
			switch (snake.direction) {
				case Up:
					DrawRectangleGradientEx({ drawRect.x, drawRect.y, drawRect.width, drawRect.height + TILE_SIZE,  }, 
						PURPLE, BLANK, BLANK, PURPLE
					);
				break;
				case Down: 
					DrawRectangleGradientEx({ drawRect.x, drawRect.y - TILE_SIZE, drawRect.width, drawRect.height + TILE_SIZE, }, 
						BLANK, PURPLE, PURPLE, BLANK
					);
				break;
				case Left:
					DrawRectangleGradientEx({ drawRect.x, drawRect.y, drawRect.width + TILE_SIZE, drawRect.height,  }, 
						PURPLE, PURPLE, BLANK, BLANK
					);
				break;
				case Right:
					DrawRectangleGradientEx({ drawRect.x - TILE_SIZE, drawRect.y, drawRect.width + TILE_SIZE, drawRect.height,  }, 
						BLANK, BLANK, PURPLE, PURPLE
					);
				break;
			}

		}

		copy.pop();
		if (copy.size() == 2) thirdToLast = tile;
		lastDrawn = tile;
	}

	DrawTextEx(GetFontDefault(),
		(std::string("Score: ") + std::to_string(snake.score)).c_str(),
		{ 25.f, 15.f, }, 25.f, 2.5f, BLACK
	);
}

void Snake::step(Grid& grid, Apple& applePos) {

	static std::random_device rd;
	static std::mt19937 e{rd()};
	static std::uniform_int_distribution<int> dist{0, GRID_SIZE - 1};
	
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
	) {
		applePos = { dist(e), dist(e), };
		score++;
	}
	else {
		tiles.pop();
		tileFromIndices(back, grid).filled = false;
	}
}

void gameStep(Grid& grid, Snake& snake, Apple& applePos) {
	snake.step(grid, applePos);
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

		// to make it return true for the collision when its not on the same tile but the head is gonna touch the rect on the next step
		// so if its the tile the head is gonna be in the next step its gonna treat it as a loss already
		switch (snake.direction) {
			case Up:    tile[1] += 1; break;
			case Down:  tile[1] -= 1; break;
			case Left:  tile[0] += 1; break;
			case Right: tile[0] -= 1; break;
		}

		if (tile == head) return true;
	}

	return lose;
}