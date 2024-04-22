#include <bave/app.hpp>
#include <bave/core/error.hpp>
#include <bave/driver.hpp>
#include <capo/error_handler.hpp>
#include <filesystem>

namespace bave {
namespace fs = std::filesystem;

App::App(std::string tag)
	: m_log{std::move(tag)}, m_bootloader([](App& app) { return std::make_unique<Driver>(app); }), m_audio_device(std::make_unique<AudioDevice>()),
	  m_audio_streamer(std::make_unique<AudioStreamer>(*m_audio_device)) {
	log::get_thread_id(); // set thread 0

	for (int i = 0; i < static_cast<int>(m_gamepads.size()); ++i) { m_gamepads.at(static_cast<GamepadId>(i)).id = GamepadId{i}; }
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

		do_wait_render_device_idle();
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
	do_shutdown();
}

auto App::set_framebuffer_size(glm::ivec2 const size) -> bool {
	auto const dr = get_display_ratio();
	if (!is_positive(dr)) { return false; }
	auto const w_size = glm::vec2{size} / dr;
	return do_set_window_size(w_size);
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

auto App::make_uri(std::string_view const full_path) const -> std::string {
	if (full_path.empty()) { return {}; }
	auto const assets_path = get_assets_path();
	if (assets_path.empty()) { return std::string{full_path}; }
	return fs::path{full_path}.lexically_relative(assets_path).generic_string();
}

auto App::get_features() const -> FeatureFlags {
	auto ret = do_get_native_features();
	if (get_render_device().validation_layers_enabled()) { ret.set(Feature::eValidationLayers); }
	return ret;
}

auto App::get_display_ratio() const -> glm::vec2 {
	auto const w_size = get_window_size();
	auto const f_size = get_framebuffer_size();
	if (w_size == f_size) { return glm::vec2{1.0f}; }
	if (!is_positive(w_size)) { return {}; }
	return glm::vec2{f_size} / glm::vec2{w_size};
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
	if (auto const* pointer_tap = std::get_if<PointerTap>(&event)) {
		m_gesture_recognizer.on_tap(*pointer_tap);
	} else if (auto const* key_input = std::get_if<KeyInput>(&event)) {
		auto const index = static_cast<std::size_t>(key_input->key);
		if (index < m_key_state.held_keys.size()) {
			auto key_state = m_key_state.held_keys[key_input->key];
			switch (key_input->action) {
			case Action::ePress: key_state = true; break;
			case Action::eRelease: key_state = false; break;
			default: break;
			}
		}
	}
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
