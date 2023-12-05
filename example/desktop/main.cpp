#include <bave/desktop_app.hpp>
#include <flappy.hpp>

auto main(int argc, char** argv) -> int {
	auto const create_info = bave::DesktopApp::CreateInfo{
		.args = bave::make_args(argc, argv),
		.title = "BaveExample",
	};

	auto app = bave::DesktopApp{create_info};
	app.set_game(std::make_unique<Flappy>(app));
	return static_cast<int>(app.run());
}
