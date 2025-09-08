#include "snake.hpp"
#include "basics.hpp"
#include "game.hpp"
#include "utils.hpp"
#include <cstdint>
#include <random>
#include <raylib.h>

Snake::Snake(size_t startLength, Game& game) :
	m_direction(Up), m_game(game), m_startLength(startLength) { }

void Snake::reset() {
	static std::uniform_int_distribution<int> dist{Up, Right};
	m_direction = static_cast<Direction>(dist(mt));

	m_tiles.clear();
	m_tiles.push_back(getRandomTile());
	for (size_t i = 1; i < m_startLength; i++) m_tiles.push_back(getNextTile());
}

Tile Snake::getNextTile() const {
	Tile ret = m_tiles.back();
	switch (m_direction) {
		case Up:    ret[1]--; break;
		case Down:  ret[1]++; break;
		case Left:  ret[0]--; break;
		case Right: ret[0]++; break;
		default: break;
	}
	if (ret[0] < 0)          ret[0] += GRID_SIZE;
	if (ret[0] >= GRID_SIZE) ret[0] -= GRID_SIZE;
	if (ret[1] < 0)          ret[1] += GRID_SIZE;
	if (ret[1] >= GRID_SIZE) ret[1] -= GRID_SIZE;

	return ret;
}

Rectangle Snake::drawTile(int id) const {
	Tile const& tile = m_tiles[id];
	Rectangle const& originalRec = tileToRec(tile);
	bool willWraparound = false;
	bool didWraparound = false;
	Rectangle drawRec = {
		originalRec.x + TILE_EDGE_SIZE, originalRec.y + TILE_EDGE_SIZE,
		USED_TILE_SPACE, USED_TILE_SPACE,
	}; // just a square in the middle of the tile unless changed later
	{
		Direction forwardsDirection = None;
		if (id < m_tiles.size() - 1) {
			Tile const& nextTile = m_tiles[id + 1];
			Tile const nextDiff = {
				static_cast<int8_t>(nextTile[0] - tile[0]),
				static_cast<int8_t>(nextTile[1] - tile[1])
			};
			if (abs(nextDiff[0]) > 1 || abs(nextDiff[1]) > 1) willWraparound = true;
			forwardsDirection = vecToDirection(nextDiff);
		}
		Direction backwardsDirection = None;
		if (id > 0) { // only the first tile doesnt connect backwards
			Tile const& prevTile = m_tiles[id - 1];
			Tile const prevDiff = {
				static_cast<int8_t>(tile[0] - prevTile[0]),
				static_cast<int8_t>(tile[1] - prevTile[1])
			};
			if (abs(prevDiff[0]) > 1 || abs(prevDiff[1]) > 1) didWraparound = true;
			backwardsDirection = vecToDirection(prevDiff);
		}
		if (didWraparound) { // it should connect to the border backwards
			switch (backwardsDirection) {
				case Down:  drawRec.y      -= TILE_EDGE_SIZE;
				case Up:    drawRec.height += TILE_EDGE_SIZE; break;
				case Right: drawRec.x      -= TILE_EDGE_SIZE;
				case Left:  drawRec.width  += TILE_EDGE_SIZE; break;
				default: break;
			}
		} else if (id > 0) { // it should connect to the previous tile
			switch (backwardsDirection) {
				case Up:    drawRec.y       -= 2 * TILE_EDGE_SIZE;
				case Down:  drawRec.height  += 2 * TILE_EDGE_SIZE; break;
				case Left:  drawRec.x       -= 2 * TILE_EDGE_SIZE;
				case Right: drawRec.width   += 2 * TILE_EDGE_SIZE; break;
				default: break;
			}
		}
		static Color constexpr MAIN_SNAKE_COLOR = { 0, 70, 145, 255 };
		DrawRectangleRec(drawRec, MAIN_SNAKE_COLOR);
		TraceLog(LOG_DEBUG, "");
		TraceLog(LOG_DEBUG, "Tile %i: (%i, %i)", id, tile[0], tile[1]);
		TraceLog(LOG_DEBUG, "\t\tForwards direction:   %s", id < m_tiles.size() - 1 ? directionToString(forwardsDirection).c_str() : "undefined");
		TraceLog(LOG_DEBUG, "\t\tBackwards direction:  %s", id > 0 ? directionToString(backwardsDirection).c_str() : "undefined");
		TraceLog(LOG_DEBUG, "\t\tDraw rec: %f, %f, %f, %f", drawRec.x, drawRec.y, drawRec.width, drawRec.height);
		if (willWraparound) { // it should connect to the border forwards
			Rectangle forwardsRec;
			switch (forwardsDirection) {
				case Up: forwardsRec = {
					originalRec.x + TILE_EDGE_SIZE,
					originalRec.y,
					USED_TILE_SPACE,
					TILE_EDGE_SIZE,
				}; break;
				case Down: forwardsRec = {
					originalRec.x + TILE_EDGE_SIZE,
					drawRec.y + drawRec.height,
					USED_TILE_SPACE,
					TILE_EDGE_SIZE,
				}; break;
				case Left: forwardsRec = {
					originalRec.x,
					originalRec.y + TILE_EDGE_SIZE,
					TILE_EDGE_SIZE,
					USED_TILE_SPACE,
				}; break;
				case Right: forwardsRec = {
					drawRec.x + drawRec.width,
					originalRec.y + TILE_EDGE_SIZE,
					TILE_EDGE_SIZE,
					USED_TILE_SPACE,
				}; break;
				default: break;
			}
			DrawRectangleRec(forwardsRec, MAIN_SNAKE_COLOR);
			TraceLog(LOG_DEBUG,
				"\t\tForwards rec: %f, %f, %f, %f",
				forwardsRec.x, forwardsRec.y,
				forwardsRec.width, forwardsRec.height
			);
		}
	}
	return drawRec;
}
void Snake::drawHead(int id) const {
	Tile const& head = m_tiles.back();
	Rectangle gradientRec = drawTile(id);
	Tile const& thirdToLast = m_tiles[id - 2];
	// i tried to add a corner gradient to make it look better but it didnt work so
	// if the snake just turned then the gradient will be shorter so that it wont look cut off
	if (head[0] == thirdToLast[0] || head[1] == thirdToLast[1]) {
		switch (m_direction) {
			case Down:  if (head[1] > 0)           { gradientRec.height += TILE_SIZE; gradientRec.y -= TILE_SIZE; } break;
			case Up:    if (head[1] < GRID_SIZE - 1) gradientRec.height += TILE_SIZE;                               break;
			case Right: if (head[0] > 0)           { gradientRec.width  += TILE_SIZE; gradientRec.x -= TILE_SIZE; } break;
			case Left:  if (head[0] < GRID_SIZE - 1) gradientRec.width  += TILE_SIZE;                               break;
			default: break;
		}
	}
	static Color const GRADIENT_COLOR = ColorLerp(BLUE, SKYBLUE, .3f);
	switch (m_direction) {
		case Up:    DrawRectangleGradientEx(gradientRec, GRADIENT_COLOR, BLANK, BLANK, GRADIENT_COLOR); break;
		case Down:  DrawRectangleGradientEx(gradientRec, BLANK, GRADIENT_COLOR, GRADIENT_COLOR, BLANK); break;
		case Left:  DrawRectangleGradientEx(gradientRec, GRADIENT_COLOR, GRADIENT_COLOR, BLANK, BLANK); break;
		case Right: DrawRectangleGradientEx(gradientRec, BLANK, BLANK, GRADIENT_COLOR, GRADIENT_COLOR); break;
		default: break;
	}
}
void Snake::draw() const {
	Tile const& head = m_tiles.back();
	for (size_t i = 0; i < m_tiles.size(); i++) {
		if (i != m_tiles.size() - 1) (void)drawTile(i);
		else drawHead(i);
		Rectangle originalRect = tileToRec(m_tiles[i]);
		Vector2 textSize = MeasureTextEx(
			GetFontDefault(),
			TextFormat("%i", i), 25.f, 2.5f
		);
	#ifdef SHOW_TILE_NUMBERS
		DrawTextEx(GetFontDefault(),
			TextFormat("%i", i), {
				originalRect.x + (originalRect.width  - textSize.x) / 2,
				originalRect.y + (originalRect.height - textSize.y) / 2
			}, 25.f, 2.5f, { 200, 200, 200, 200 }
		);
	#endif
	}
}
