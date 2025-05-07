#include <raylib.h>

int main() {

	int constexpr screenWidth = 1800;
	int constexpr screenHeight = 900;

	InitWindow(screenWidth, screenHeight, "Hello!");

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(SKYBLUE);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}