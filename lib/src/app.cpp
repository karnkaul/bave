#include <bave/app.hpp>
#include <bave/game.hpp>

namespace bave {
App::App(std::string tag) : m_log(std::move(tag)), m_game_factory([](App& app) { return std::make_unique<Game>(app); }) {}

void App::set_game_factory(std::function<std::unique_ptr<class Game>(App&)> game_factory) {
	if (!game_factory) { return; }
	m_game_factory = std::move(game_factory);
}

auto App::run() -> ErrCode {
	try {
		m_log.info("run");
		do_run();
		m_log.info("exit");
		return ErrCode::eSuccess;
	} catch (std::runtime_error const& e) {
		m_log.error("FATAL: {}", e.what());
		return ErrCode::eFailure;
	} catch (...) {
		m_log.error("PANIC");
		return ErrCode::eFailure;
	}
}

void App::shutdown() {
	m_log.info("shutdown requested");
	m_shutting_down = true;
	do_shutdown();
}

void App::start_next_frame() {
	m_events.clear();
	m_dt.update();
}

auto App::make_game() -> std::unique_ptr<Game> {
	assert(m_game_factory);
	return m_game_factory(*this);
}
} // namespace bave
