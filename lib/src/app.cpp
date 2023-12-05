#include <bave/app.hpp>
#include <bave/game.hpp>

namespace bave {
void App::Deleter::operator()(Game* ptr) const noexcept { std::default_delete<Game>{}(ptr); }

App::App(std::string tag) : m_log(std::move(tag)) { m_game = std::unique_ptr<Game, Deleter>(new Game{*this}); }

void App::set_game(std::unique_ptr<Game> game) {
	if (!game) { return; }
	m_game.reset(game.release());
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
	m_game->shutdown();
	do_shutdown();
}
} // namespace bave
