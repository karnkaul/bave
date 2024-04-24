#include <bave/desktop_app.hpp>
#include <src/flappy.hpp>

// desktop entry point
auto main(int argc, char** argv) -> int {
	// we will use bave::DesktopApp.
	// set up its CreateInfo:
	// create a data loader by searching for assets in a super directory of the exe dir.
	// this allows debugging without the need to set the working directory to <project_root>/example.
	auto data_loader = bave::DataLoaderBuilder{argc, argv}.add_dir("assets,example/assets").build();
	auto create_info = bave::DesktopApp::CreateInfo{
		.title = "BaveExample",
		.mode = bave::Windowed{.extent = {720, 1280}},
		.msaa = vk::SampleCountFlagBits::e4,
		// pass the custom data loader.
		.data_loader = std::move(data_loader),
	};

	// create the App instance.
	auto app = bave::DesktopApp{std::move(create_info)};
	// setup the entry point (Flappy).
	app.set_bootloader([](bave::App& app) { return std::make_unique<Flappy>(app); });
	// run App and return its exit code.
	// after its own setup, App will create Flappy (via the factory passed above) and drive it every frame.
	return static_cast<int>(app.run());
}
