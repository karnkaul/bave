#include <flappy.hpp>
#include <cmath>

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

	m_elapsed += get_app().get_dt();
	m_clear_red = 0.5f * std::sin(m_elapsed.count()) + 0.5f;
}
