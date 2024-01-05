#include <bave/app.hpp>
#include <bave/core/error.hpp>
#include <bave/game.hpp>
#include <capo/error_handler.hpp>

namespace bave {
App::App(std::string tag)
	: m_log{std::move(tag)}, m_game_factory([](App& app) { return std::make_unique<Game>(app); }), m_audio_device(std::make_unique<AudioDevice>()),
	  m_audio_streamer(std::make_unique<AudioStreamer>(*m_audio_device)) {
	log::get_thread_id(); // set thread 0
}

void App::set_game_factory(std::function<std::unique_ptr<class Game>(App&)> game_factory) {
	if (!game_factory) {
		m_log.error("cannot set null game factory");
		return;
	}

	m_game_factory = std::move(game_factory);
}

void App::set_data_store(std::unique_ptr<DataStore> data_store) {
	if (!data_store) {
		m_log.error("cannot set null DataStore");
		return;
	}

	m_data_store = std::move(data_store);
}

auto App::run() -> ErrCode {
	try {
		if (auto const ret = setup()) { return *ret; }

		while (!is_shutting_down()) {
			start_next_frame();
			poll_events();
			pre_tick();
			tick();
			render();
		}

		get_render_device().get_device().waitIdle();
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

auto App::load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader> {
	auto const& renderer = get_renderer();
	if (!renderer.is_rendering()) {
		m_log.error("can only load shaders when rendering");
		return {};
	}

	auto& shader_cache = renderer.get_pipeline_cache().get_shader_cache();
	auto vert = shader_cache.load(vertex);
	auto frag = shader_cache.load(fragment);
	if (!vert || !frag) { return {}; }

	return Shader{&get_renderer(), vert, frag};
}

void App::start_next_frame() {
	m_events.clear();
	m_drops.clear();
	m_dt.update();
}

void App::pre_tick() {
	m_gesture_recognizer.update(get_active_pointers());
	m_audio_streamer->tick(get_dt());
}

void App::push_event(Event event) {
	if (auto const* pointer_tap = std::get_if<PointerTap>(&event)) { m_gesture_recognizer.on_tap(*pointer_tap); }
	m_events.push_back(event);
}

void App::push_drop(std::string path) { m_drops.push_back(std::move(path)); }

auto App::make_game() -> std::unique_ptr<Game> {
	assert(m_game_factory);
	auto ret = m_game_factory(*this);
	m_dt.update();
	return ret;
}

auto App::screen_to_framebuffer(glm::vec2 const position) const -> glm::vec2 {
	glm::vec2 const window_size = get_window_size();
	if (window_size.x <= 0.0f || window_size.y <= 0.0f) { return position; }

	auto const offset = 0.5f * window_size;
	auto const centred = glm::vec2{position.x - offset.x, offset.y - position.y};
	auto const normalized = centred / window_size;
	return normalized * glm::vec2{get_framebuffer_size()};
}
} // namespace bave
