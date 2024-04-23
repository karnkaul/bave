#include <bave/desktop_app.hpp>
#include <tools/runner.hpp>

auto main(int argc, char** argv) -> int {
	using namespace bave::tools;

	auto const args = bave::MainArgs{argc, argv};
	auto assets_path = args.upfind_assets_dir("assets,example/assets");
	auto daci = bave::DesktopApp::CreateInfo{
		.args = args,
		.title = "Bave Tools",
		.mode = bave::Windowed{.extent = Applet::window_size_v, .lock_aspect_ratio = false},
		.data_loader = std::make_unique<bave::FileLoader>(assets_path),
	};
	auto app = bave::DesktopApp{std::move(daci)};
	app.set_bootloader(Bootloader{std::move(assets_path)});
	return static_cast<int>(app.run());
}
