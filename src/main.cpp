#include "game.hpp"
int main(int argc, char* argv[]) {
	Game game{};
DESKTOP_ONLY(
	std::optional<int> result = game.handleCli(argc, argv);
	if (result) return *result;
)
	game.init();
	while (game.run()) continue;
	game.deinit();
	return 0;
}
