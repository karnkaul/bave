#include <bave/game.hpp>
#include <bave/android_app.hpp>

namespace {
struct Game : bave::Game {
	using bave::Game::Game;
};
} // namespace

extern "C" {
void android_main(android_app* andr_app) {
	auto app = bave::AndroidApp{*andr_app};
	app.set_game(std::make_unique<Game>(app));
	app.run();
}
}
