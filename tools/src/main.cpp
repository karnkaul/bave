#include <bave/desktop_app.hpp>
#include <tools/runner.hpp>

auto main(int argc, char** argv) -> int {
	using namespace bave::tools;

	auto const args = bave::MainArgs{argc, argv};
	auto const daci = bave::DesktopApp::CreateInfo{
		.args = args,
		.title = "Bave Tools",
		.mode = bave::Windowed{.extent = Applet::window_size_v, .lock_aspect_ratio = false},
		.assets_dir = args.find_assets_super_dir("assets,example/assets"),
	};
	auto app = bave::DesktopApp{daci};
	app.set_bootloader(Bootloader{});
	return static_cast<int>(app.run());
}
