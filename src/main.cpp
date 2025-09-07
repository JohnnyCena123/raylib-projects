#include "game.hpp"
int main(int argc, char* argv[]) {
	std::optional<int> result = std::nullopt;
	Game game{argc, argv, result};
	if (result) return *result;

	game.run();

	return 0;
}
