#pragma once

#include <deque>
#include <raylib.h>

#include "basics.hpp"

class Game;

class Snake {

	friend class Game;

public:
	Snake() = delete;
	Snake(size_t startLength, Game& game);

	void reset();
	Tile getNextTile() const;

	void draw() const;

private:
	size_t m_startLength;
	std::deque<Tile> m_tiles;

	Direction m_direction;

	Game& m_game;

	Rectangle drawTile(int id) const;
	void drawHead(int id) const;
};
