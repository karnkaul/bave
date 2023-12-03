#include <bave/desktop_app.hpp>
#include <bave/game.hpp>

namespace {
struct Game : bave::Game {
	using bave::Game::Game;
};
} // namespace

auto main(int argc, char** argv) -> int {
	auto app = bave::DesktopApp{argc, argv};
	app.set_game(std::make_unique<Game>(app));
	return static_cast<int>(app.run());
}
