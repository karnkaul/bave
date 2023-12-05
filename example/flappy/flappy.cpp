#include <flappy.hpp>

void Flappy::tick() {
	for (auto const& event : get_app().get_events()) {
		if (auto const* focus_change = std::get_if<bave::FocusChange>(&event)) {
			m_log.info("focus {}", focus_change->in_focus ? "gained" : "lost");
			//
		}

		if (auto const* key_input = std::get_if<bave::KeyInput>(&event)) {
			if (key_input->key == bave::Key::eW && key_input->mods.test(bave::mod::ctrl)) {
				m_log.info("shutting down");
				get_app().shutdown();
			}

			if (key_input->key == bave::Key::eEscape && key_input->action == bave::Action::eRelease) {
				m_log.info("shutting down");
				get_app().shutdown();
			}
		}

		if (auto const* mouse_click = std::get_if<bave::MouseClick>(&event)) {
			m_log.info("tap {} at {}x{}", (mouse_click->action == bave::Action::eRelease ? "up" : "down"), mouse_click->position.x, mouse_click->position.y);
		}
	}

	auto const fb_size = get_app().get_framebuffer_size();
	if (fb_size != m_fb_size) {
		m_fb_size = fb_size;
		m_log.debug("Framebuffer size: {}x{}", fb_size.x, fb_size.y);
	}

	m_elapsed += get_app().get_dt();
	if (m_elapsed >= 3s) {
		// get_app().shutdown();
	}
}
