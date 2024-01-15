#include <bave/desktop_app.hpp>
#include <bave/logger.hpp>
#include <tools/runner.hpp>

auto main(int argc, char** argv) -> int {
	using namespace bave::tools;

	auto const daci = bave::DesktopApp::CreateInfo{
		.args = bave::make_args(argc, argv),
		.title = "Bave Tools",
		.extent = Applet::window_size_v,
		.lock_aspect_ratio = false,
		.assets_patterns = "assets,example/assets",
	};
	auto app = bave::DesktopApp{daci};
	app.set_bootloader(Bootloader{});
	return static_cast<int>(app.run());
}
