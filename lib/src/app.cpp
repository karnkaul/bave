#include <bave/app.hpp>
#include <bave/core/error.hpp>
#include <bave/driver.hpp>
#include <capo/error_handler.hpp>

namespace bave {
App::App(std::string tag)
	: m_log{std::move(tag)}, m_bootloader([](App& app) { return std::make_unique<Driver>(app); }), m_audio_device(std::make_unique<AudioDevice>()),
	  m_audio_streamer(std::make_unique<AudioStreamer>(*m_audio_device)) {
	log::get_thread_id(); // set thread 0
}

void App::set_bootloader(Bootloader bootloader) {
	if (!bootloader) {
		m_log.error("cannot set null driver factory");
		return;
	}

	m_bootloader = std::move(bootloader);
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
	} catch (std::exception const& e) {
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

auto App::set_framebuffer_size(glm::ivec2 const size) -> bool {
	auto const dr = get_display_ratio();
	if (!is_positive(dr)) { return false; }
	auto const w_size = glm::vec2{size} / dr;
	return do_set_window_size(w_size);
}

auto App::get_display_ratio() const -> glm::vec2 {
	auto const w_size = get_window_size();
	auto const f_size = get_framebuffer_size();
	if (w_size == f_size) { return glm::vec2{1.0f}; }
	if (!is_positive(w_size)) { return {}; }
	return glm::vec2{f_size} / glm::vec2{w_size};
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

// NOLINTNEXTLINE(readability-make-member-function-const)
auto App::change_mount_point(std::string_view const directory) -> bool {
	if (!get_data_store().set_mount_point(directory)) { return false; }
	get_renderer().get_pipeline_cache().clear_loaded();
	return true;
}

void App::start_next_frame() {
	m_events.clear();
	m_drops.clear();
	m_dt.update();
}

void App::pre_tick() {
	m_gesture_recognizer.update(get_active_pointers());
	m_audio_streamer->tick(get_dt());
	m_timer.tick(get_dt());
}

void App::push_event(Event event) {
	if (auto const* pointer_tap = std::get_if<PointerTap>(&event)) { m_gesture_recognizer.on_tap(*pointer_tap); }
	m_events.push_back(event);
}

void App::push_drop(std::string path) { m_drops.push_back(std::move(path)); }

auto App::boot_driver() -> std::unique_ptr<Driver> {
	assert(m_bootloader);
	auto ret = m_bootloader(*this);
	if (!ret) { throw Error{"failed to boot Driver"}; }
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
