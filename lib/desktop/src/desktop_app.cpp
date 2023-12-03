#include <bave/desktop_app.hpp>
#include <bave/logger.hpp>

namespace bave {
DesktopApp::DesktopApp(int argc, char const* const* argv) : m_args(argv, static_cast<size_t>(argc)) { m_game = std::make_unique<Game>(*this); }

void DesktopApp::set_game(std::unique_ptr<Game> game) {
	if (!game) { return; }
	m_game = std::move(game);
}

auto DesktopApp::run() -> ErrCode {
	m_log.log(bave::log::info_v, "run");
	//
	m_log.log(log::info_v, "exit");
	return ErrCode::eSuccess;
}
} // namespace bave
