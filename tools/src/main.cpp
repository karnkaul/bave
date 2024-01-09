#include <bave/desktop_app.hpp>
#include <bave/logger.hpp>
#include <tools/nine_slicer.hpp>

namespace {
using namespace bave::tools;

[[nodiscard]] auto load_or_create_state() {
	static auto const logger = bave::Logger{"BaveTools"};
	auto ret = State{};
	if (ret.load()) {
		logger.info("loaded State from '{}'", State::path_v.as_view());
	} else {
		ret.save();
		logger.info("created State at '{}'", State::path_v.as_view());
	}
	return ret;
}
} // namespace

auto main(int argc, char** argv) -> int {
	Applet::add_applet<NineSlicer>("NineSlicer");

	auto const state = load_or_create_state();

	auto const daci = bave::DesktopApp::CreateInfo{
		.args = bave::make_args(argc, argv),
		.title = "Bave Tools",
		.extent = {600, 600},
		.lock_aspect_ratio = false,
		.assets_patterns = "assets,example/assets",
	};
	auto app = bave::DesktopApp{daci};
	app.set_bootloader(Applet::make_bootloader(state));
	return static_cast<int>(app.run());
}
