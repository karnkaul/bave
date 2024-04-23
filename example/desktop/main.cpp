#include <bave/desktop_app.hpp>
#include <src/flappy.hpp>

// desktop entry point
auto main(int argc, char** argv) -> int {
	// we will use bave::DesktopApp.
	// set up its CreateInfo:
	// create MainArgs, use it to search for assets.
	auto const args = bave::MainArgs{argc, argv};
	// search for assets in a super directory of the exe dir.
	// this allows debugging without the need to set the working directory to <project_root>/example.
	auto const assets_path = args.upfind_assets_dir("assets,example/assets");
	auto create_info = bave::DesktopApp::CreateInfo{
		.args = args,
		.title = "BaveExample",
		.mode = bave::Windowed{.extent = {720, 1280}},
		.msaa = vk::SampleCountFlagBits::e4,
		// pass a custom data loader that uses the located assets path.
		.data_loader = std::make_unique<bave::FileLoader>(assets_path),
	};

	// create the App instance.
	auto app = bave::DesktopApp{std::move(create_info)};
	// setup the entry point (Flappy).
	app.set_bootloader([](bave::App& app) { return std::make_unique<Flappy>(app); });
	// run App and return its exit code.
	// after its own setup, App will create Flappy (via the factory passed above) and drive it every frame.
	return static_cast<int>(app.run());
}
