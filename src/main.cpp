#include <array>
#include <format>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <string>


/*
 * Constants
 */
float constexpr SQUARE_SIZE = 200.f;
float constexpr Y_FREE_SPACE = 75.f;
int constexpr SCREEN_WIDTH = 6 * SQUARE_SIZE;
int constexpr SCREEN_HEIGHT = 3 * SQUARE_SIZE + Y_FREE_SPACE * 2;
Vector2 constexpr BOARD_START = { SQUARE_SIZE * 1.5f, Y_FREE_SPACE, };
Color constexpr SQUARE_COLOR = { 155, 228, 255, 115, };
	 

using Player = enum { None, X, O, };

std::array<std::array<std::pair<Rectangle, Player>, 3>, 3> squares{};

void draw() {

	DrawLineV(
		{ SQUARE_SIZE + BOARD_START.x, BOARD_START.y, },
		{ SQUARE_SIZE + BOARD_START.x, SQUARE_SIZE * 3.f + BOARD_START.y, }, BLACK
	); // left line
	DrawLineV(
		{ SQUARE_SIZE * 2 + BOARD_START.x, BOARD_START.y, },
		{ SQUARE_SIZE * 2 + BOARD_START.x, SQUARE_SIZE * 3.f + BOARD_START.y, }, BLACK
	); // right line

	DrawLineV(
		{ BOARD_START.x, SQUARE_SIZE + BOARD_START.y, },
		{ SQUARE_SIZE * 3.f + BOARD_START.x, SQUARE_SIZE + BOARD_START.y, }, BLACK
	); // top line
	DrawLineV(
		{ BOARD_START.x, SQUARE_SIZE * 2 + BOARD_START.y, },
		{ SQUARE_SIZE * 3 + BOARD_START.x, SQUARE_SIZE * 2 + BOARD_START.y, }, BLACK
	); // bottom line

	for (auto const& line : squares) for (auto const& [rect, item] : line) {
		DrawRectangleRec(rect, SQUARE_COLOR);
		switch (item) {
			case None: break;
			case X: 
				DrawLineEx(
					{ rect.x + 10.f, rect.y + 10.f, }, {
						rect.x + rect.width - 10.f, 
						rect.y + rect.height - 10.f
					}, 3.f, BLACK
				);
				DrawLineEx(
					{ rect.x + rect.width - 10.f, rect.y + 10.f, }, 
					{ rect.x + 10.f, rect.y + rect.height - 10.f, }, 3.f, BLACK
				);
				break;
			case O: 
				DrawCircleLinesV(
					{ rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, }, 
					SQUARE_SIZE / 2.f - 10.f, BLACK
				);

				break;
			default: DrawText(
				"What the hell did you do", 
				SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 
				100, RAYWHITE
			); break;
		}
	}
}

void initSquares() {
	for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
		squares[i][j] = std::make_pair(Rectangle{
			BOARD_START.x + SQUARE_SIZE * j,
			BOARD_START.y + SQUARE_SIZE * i,
			SQUARE_SIZE, SQUARE_SIZE,
		}, None);
	}

}

Player checkWin(std::array<std::array<int, 2>, 3>& winCombination) {
	// 3 for 3 required squares per win combination, 2 for indices of square in the 2D square array at line 18.
	// double braces because std::array oh well
	static std::array<std::array<std::array<int, 2>, 3>, 8> constexpr POSSIBLE_WINS = {{
		// horizontals
		{{ { 0, 0, }, { 0, 1, }, { 0, 2, }, }},
		{{ { 1, 0, }, { 1, 1, }, { 1, 2, }, }},
		{{ { 2, 0, }, { 2, 1, }, { 2, 2, }, }},
		// verticals
		{{ { 0, 0, }, { 1, 0, }, { 2, 0, }, }},
		{{ { 0, 1, }, { 1, 1, }, { 2, 1, }, }},
		{{ { 0, 2, }, { 1, 2, }, { 2, 2, }, }},
		// diagonals
		{{ { 0, 0, }, { 1, 1, }, { 2, 2, }, }},
		{{ { 0, 2, }, { 1, 1, }, { 2, 0, }, }},
	}};

	Player ret = None;
	for (auto const& winCondition : POSSIBLE_WINS) {
		Player marks[3];
		for (int i = 0; i < 3; i++) marks[i] = squares[winCondition[i][1]][winCondition[i][0]].second;
		if (marks[0] != None && marks[0] == marks[1] && marks[1] == marks[2]) {
			ret = marks[0];
			winCombination = winCondition;
			break;
		}
	}

	return ret;
}

bool startGame() {
	bool shouldContinue = false;
	
	initSquares();

	auto font = LoadFont("../resources/font.ttf");

	auto currentTurnTextImage = ImageTextEx(font, "Current turn is:", 40.f, 1.f, RED);
	auto currentTurnTextTexture = LoadTextureFromImage(currentTurnTextImage);

	auto restartBtnImage = LoadImage("../resources/restart_btn.png");
	auto restartBtnTexture = LoadTextureFromImage(restartBtnImage);

	Texture2D endText;

	static int XWinCount = 0;
	static int OWinCount = 0;

	auto winCountsTextImage = ImageTextEx(
		font, "X    :    O", 
		75.f, 1.f, RED
	);
	auto XWinCountTextImage = ImageTextEx(
		font, std::to_string(XWinCount).c_str(), 50.f, 1.f, RED
	);
	auto OWinCountTextImage = ImageTextEx(
		font, std::to_string(OWinCount).c_str(), 50.f, 1.f, RED
	);
	auto winCountsTextTexture = LoadTextureFromImage(winCountsTextImage);
	auto XWinCountTextTexture = LoadTextureFromImage(XWinCountTextImage);
	auto OWinCountTextTexture = LoadTextureFromImage(OWinCountTextImage);
	
	bool mouseHeld = false;
	bool currentTurnIsO = false;

	int turnCount = 0;
	bool hasEnded = false;
	bool isWin = false;
	double endTime = 0.;

	Vector2 lineStartPos;
	Vector2 lineEndPos;
	enum { Horizontal, Vertical, Diagonal, } winAxis;
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(SKYBLUE);

		draw();

		DrawTextureV(
			winCountsTextTexture, { 
				SCREEN_WIDTH / 2.f - winCountsTextTexture.width / 2.f, 2.f,
			}, WHITE
		);
		DrawTextureV(
			XWinCountTextTexture, {
				(SCREEN_WIDTH - XWinCountTextImage.width) / 2.f - 24.f, 
				(winCountsTextImage.height - XWinCountTextImage.height) / 2.f + 5.5f, 
			},  WHITE
		);
		DrawTextureV(
			OWinCountTextTexture, {
				(SCREEN_WIDTH - OWinCountTextImage.width) / 2.f + 16.f, 
				(winCountsTextImage.height - OWinCountTextImage.height) / 2.f + 5.5f, 
			},  WHITE
		);

		DrawTextureV(currentTurnTextTexture, {
			BOARD_START.x + SQUARE_SIZE * 3 + (BOARD_START.x - currentTurnTextTexture.width) / 2.f,
			Y_FREE_SPACE + 175.f,
		}, WHITE);

		auto currentTurnIndicatorSize = Vector2{ SQUARE_SIZE * .7f, SQUARE_SIZE * .7f, };
		auto currentTurnIndicatorRect = Rectangle{
			BOARD_START.x + SQUARE_SIZE * 3 + (BOARD_START.x - currentTurnIndicatorSize.x) / 2.f,
			(SCREEN_HEIGHT - currentTurnIndicatorSize.y) / 2.f, 
			currentTurnIndicatorSize.x, currentTurnIndicatorSize.y,
		};
		if (!currentTurnIsO) {
			DrawLineEx(
				{ currentTurnIndicatorRect.x, currentTurnIndicatorRect.y, }, {
					currentTurnIndicatorRect.x + currentTurnIndicatorRect.width, 
					currentTurnIndicatorRect.y + currentTurnIndicatorRect.height,
				}, 3.f, BLACK
			);
			DrawLineEx(
				{ currentTurnIndicatorRect.x + currentTurnIndicatorRect.width, currentTurnIndicatorRect.y, }, 
				{ currentTurnIndicatorRect.x, currentTurnIndicatorRect.y + currentTurnIndicatorRect.height, }, 3.f, BLACK
			);
		} else {
			DrawCircleLinesV({ 
					currentTurnIndicatorRect.x + currentTurnIndicatorRect.width  / 2.f, 
					currentTurnIndicatorRect.y + currentTurnIndicatorRect.height / 2.f, 
				}, currentTurnIndicatorRect.width / 2.f, BLACK
			);
		}


		Vector2 restartBtnRealSize = { restartBtnTexture.width * .35f, restartBtnTexture.height * .35f, };
		auto restartBtnRect = Rectangle{
			(BOARD_START.x - restartBtnRealSize.x) / 2.f, 
			(SCREEN_HEIGHT - restartBtnRealSize.y) / 2.f, 
			restartBtnRealSize.x, restartBtnRealSize.y,
		};
		DrawTextureEx(restartBtnTexture, { restartBtnRect.x, restartBtnRect.y, }, 0.f, .35f, WHITE);
		if (!IsMouseButtonDown(0) && mouseHeld && CheckCollisionPointRec(GetMousePosition(), restartBtnRect)) {
			shouldContinue = true;
			break;
		}
		
		// checking if the board has been filled at the previous frame
		if (turnCount == 9 && !hasEnded) {
			hasEnded = true;
			endTime = GetTime();
			auto endTextImage = ImageTextEx(
				font,
				"The game ended in a draw.",
				50.f, 1.f, RED
			);
			endText = LoadTextureFromImage(endTextImage);
			UnloadImage(endTextImage);
		}

		// drawing the end text and the line that shows where the win happened if at all
		if (hasEnded) {
			bool drawText = true;
			if (isWin) {
				float partOfLineToDraw = std::min((GetTime() - endTime) / 2., 1.);
				if (GetTime() - endTime < 3.) drawText = false;

				Vector2 lineRealEndPos = lineEndPos * partOfLineToDraw; 
				switch (winAxis) {
					case Horizontal: lineRealEndPos.y = lineEndPos.y; break;
					case Vertical:   lineRealEndPos.x = lineEndPos.x; break;
					case Diagonal:   if (lineEndPos == Vector2{ SQUARE_SIZE * 3, 0.f, }) lineRealEndPos.y = SQUARE_SIZE * 3 * (1.f - partOfLineToDraw); 
				}
				DrawLineEx(BOARD_START + lineStartPos, BOARD_START + lineRealEndPos, 15.f, RED);
			}
			DrawTextureV(
				endText, { 
					SCREEN_WIDTH / 2.f - endText.width / 2.f, 
					15.f + SQUARE_SIZE * 3 + Y_FREE_SPACE, 
				}, WHITE
			);
		}

		// performing a turn
		if (IsMouseButtonDown(0)) mouseHeld = true;
		else {
			if (!hasEnded) {
			bool checkForWin = false;
			if (mouseHeld) for (auto& line : squares) for (auto& [rect, item] : line)
				if (CheckCollisionPointRec({GetMousePosition()}, rect) && item == None) {
					item = currentTurnIsO ? O : X;
					currentTurnIsO = !currentTurnIsO;
					turnCount++;
					if (turnCount >= 5) checkForWin = true;
					break;
				}

			std::array<std::array<int, 2>, 3> winCombination = {};
			Player whoWon = None;
			if (checkForWin) whoWon = checkWin(winCombination);
				if (whoWon != None) {
					if (winCombination[0][0] == winCombination[2][0]) { // x's equal, its vertical
						winAxis = Vertical;
						lineStartPos = { SQUARE_SIZE * (winCombination[0][0] + .5f), 0.f, };
						lineEndPos   = { SQUARE_SIZE * (winCombination[0][0] + .5f), SQUARE_SIZE * 3, };
					} else if (winCombination[0][1] == winCombination[2][1]) { // y's equal, its horizontal
						winAxis = Horizontal;
						lineStartPos = { 0.f,             SQUARE_SIZE * (winCombination[0][1] + .5f), };
						lineEndPos   = { SQUARE_SIZE * 3, SQUARE_SIZE * (winCombination[0][1] + .5f), };
					} else if (winCombination[0] == std::array<int, 2>{ 0, 0, }) { // not horizontal nor vertical, it must be diagonal
						winAxis = Diagonal;
						lineStartPos = { 0.f, 0.f, };
						lineEndPos   = { SQUARE_SIZE * 3, SQUARE_SIZE * 3, };
					} else {
						winAxis = Diagonal;
						lineStartPos = { 0.f, SQUARE_SIZE * 3, };
						lineEndPos   = { SQUARE_SIZE * 3, 0.f, };
					}
					
					hasEnded = true;
					isWin = true;
					endTime = GetTime();
					auto endTextImage = ImageTextEx(font,
						std::format("{} has won!", whoWon == X ? "X" : "O").c_str(),
						50, 1, RED
					);
					endText = LoadTextureFromImage(endTextImage);
					UnloadImage(endTextImage);

					(whoWon == X ? XWinCount : OWinCount)++;

					UnloadImage(XWinCountTextImage);
					UnloadImage(OWinCountTextImage);
					XWinCountTextImage = ImageTextEx(
						font, std::to_string(XWinCount).c_str(), 50.f, 1.f, RED
					);
					OWinCountTextImage = ImageTextEx(
						font, std::to_string(OWinCount).c_str(), 50.f, 1.f, RED
					);
					XWinCountTextTexture = LoadTextureFromImage(XWinCountTextImage);
					OWinCountTextTexture = LoadTextureFromImage(OWinCountTextImage);
				}
			}
			mouseHeld = false;
		}

		EndDrawing();
	}

	UnloadFont(font);

	UnloadImage(currentTurnTextImage);
	UnloadTexture(currentTurnTextTexture);

	UnloadImage(restartBtnImage);
	UnloadTexture(restartBtnTexture);

	UnloadImage(winCountsTextImage);
	UnloadImage(XWinCountTextImage);
	UnloadImage(OWinCountTextImage);
	UnloadTexture(winCountsTextTexture);
	UnloadTexture(XWinCountTextTexture);
	UnloadTexture(OWinCountTextTexture);

	if (IsTextureValid(endText)) UnloadTexture(endText);

	return shouldContinue;
}

int main() {

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello!");
	SetTargetFPS(60);
	rlSetLineWidth(5.f);

	while (startGame()) continue;

	CloseWindow();

	return 0;
}