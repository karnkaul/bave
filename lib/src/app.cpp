#include <bave/app.hpp>
#include <bave/core/error.hpp>
#include <bave/game.hpp>

namespace bave {
App::App(std::string tag) : m_log(std::move(tag)), m_game_factory([](App& app) { return std::make_unique<Game>(app); }) {}

void App::set_game_factory(std::function<std::unique_ptr<class Game>(App&)> game_factory) {
	if (!game_factory) { return; }
	m_game_factory = std::move(game_factory);
}

void App::set_data_store(std::unique_ptr<DataStore> data_store) {
	if (!data_store) { throw Error{"Setting null DataStore"}; }
	m_data_store = std::move(data_store);
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

auto App::get_data_store() const -> DataStore& {
	if (!m_data_store) { throw Error{"Dereferencing null DataStore"}; }
	return *m_data_store;
}

auto App::load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader> {
	auto const& frame_renderer = get_frame_renderer();
	if (!frame_renderer.is_rendering()) {
		m_log.error("can only load shaders when rendering");
		return {};
	}

	auto& shader_cache = frame_renderer.get_pipeline_cache().get_shader_cache();
	auto vert = shader_cache.load(vertex);
	auto frag = shader_cache.load(fragment);
	if (!vert || !frag) { return {}; }

	return Shader{&get_frame_renderer(), {.vertex = vert, .fragment = frag}, render_view};
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
