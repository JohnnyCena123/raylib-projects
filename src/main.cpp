#include <raylib.h>

int constexpr SCREEN_WIDTH = 1800;
int constexpr SCREEN_HEIGHT = 900;

int main() {

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello!");

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(SKYBLUE);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}