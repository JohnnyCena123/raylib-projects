#include "game.hpp"

int main(int argc, char* argv[]) {

	Game game{};
DESKTOP_ONLY(
	game.handleCli(argc, argv);
)

	game.init();
	game.run();
	game.deinit();

	return 0;
}