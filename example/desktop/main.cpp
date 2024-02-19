#include <bave/desktop_app.hpp>
#include <src/flappy.hpp>

// desktop entry point
auto main(int argc, char** argv) -> int {
	// we will use bave::DesktopApp.
	// set up its CreateInfo:
	auto const create_info = bave::DesktopApp::CreateInfo{
		.args = bave::make_args(argc, argv),
		.title = "BaveExample",
		.mode = bave::Windowed{.extent = {720, 1280}},
		.msaa = vk::SampleCountFlagBits::e4,
		.assets_patterns = "assets,example/assets",
	};

	// create the App instance.
	auto app = bave::DesktopApp{create_info};
	// setup the entry point (Flappy).
	app.set_bootloader([](bave::App& app) { return std::make_unique<Flappy>(app); });
	// run App and return its exit code.
	// after its own setup, App will create Flappy (via the factory passed above) and drive it every frame.
	return static_cast<int>(app.run());
}
