#include <bave/graphics/pixmap.hpp>
#include <bave/graphics/projector.hpp>
#include <bave/loader.hpp>
#include <flappy.hpp>

namespace {
constexpr auto world_space_v = glm::vec2{1440.0f, 2560.0f};
}

Flappy::Flappy(bave::App& app) : Game(app), m_text(&app.get_render_device()) {

	auto loader = bave::Loader{&app.get_data_store(), &app.get_render_device()};

	if (auto font = loader.load_font("fonts/Vera.ttf")) { m_text.set_font(std::move(font)); }

	m_sheet = loader.load_sprite_sheet("images/player_sheet.json");
	m_animation = loader.load_sprite_animation("animations/player_anim.json");
	m_sprite = bave::AnimatedSprite{&app.get_render_device(), m_sheet, 0.5s};
	m_sprite->set_size(glm::vec2{300.0f});
	m_sprite->animation = *m_animation;
	// m_sprite->repeat = false;

	m_text.set_string("hello");
	// m_text.set_align(bave::Text::Align::eLeft);
	m_text.transform.position.y = 500.0f;

	m_game_view = get_app().get_render_device().render_view;
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_width(world_space_v);
}

void Flappy::tick() {
	m_elapsed += get_app().get_dt();
	m_clear_red = 0.5f * std::sin(m_elapsed.count()) + 0.5f;
	clear_colour = bave::Rgba::from({m_clear_red, 0.0f, 0.0f, 1.0f});

	auto const& gesture_recognizer = get_app().get_gesture_recognizer();

	if (auto const pinch = gesture_recognizer.pinch_delta()) { m_game_view.transform.scale += 0.1f * *pinch * get_app().get_dt().count(); }

	if (auto const drag = gesture_recognizer.drag_position()) {
		auto const delta = get_app().get_render_device().project_to(world_space_v, *drag - m_prev_pointer);
		m_game_view.transform.position -= delta;
	}

	if (auto const tap_up = gesture_recognizer.tap_up()) { m_target = m_game_view.unproject(*tap_up / glm::vec2{get_app().get_framebuffer_size()}); }

	auto const move_delta = m_target - m_sprite->transform.position;
	if (glm::length2(move_delta) > 100.0f) {
		m_sprite->transform.position += 1000.0f * glm::normalize(move_delta) * get_app().get_dt().count();
		m_sprite->transform.scale.x = move_delta.x < 0.0f ? -1.0f : 1.0f;
	}

	m_sprite->tick(get_app().get_dt());

	m_prev_pointer = m_pointer;

	IFBAVEIMGUI({
		ImGui::ShowDemoWindow();
		// ...
	});
}

void Flappy::render() const {
	get_app().get_render_device().render_view = m_game_view;
	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) {
		m_sprite->draw(*shader);
		m_text.draw(*shader);
	}
}

void Flappy::on_key(bave::KeyInput const key_input) {
	if (key_input.key == bave::Key::eW && key_input.mods.test(bave::mod::ctrl)) {
		m_log.info("shutting down");
		get_app().shutdown();
	}

	if (key_input.key == bave::Key::eEscape && key_input.action == bave::Action::eRelease) {
		m_log.info("shutting down");
		get_app().shutdown();
	}
}

void Flappy::on_move(bave::PointerMove const pointer_move) {
	auto const& pointer = pointer_move.pointer;
	if (pointer.id == bave::Pointer::Id::ePrimary) { m_pointer = pointer.position; }
}

void Flappy::on_scroll(bave::MouseScroll const mouse_scroll) { m_game_view.transform.scale += 0.1f * mouse_scroll.delta.y; }
