#include <bave/graphics/pixmap.hpp>
#include <bave/graphics/projector.hpp>
#include <bave/loader.hpp>
#include <src/flappy.hpp>
#include <thread>

using bave::Action;
using bave::AnimatedSprite;
using bave::App;
using bave::FocusChange;
using bave::Key;
using bave::KeyInput;
using bave::Loader;
using bave::PointerTap;
using bave::Rect;
using bave::Rgba;
using bave::Seconds;
using bave::Text;
using bave::Texture;

Flappy::Flappy(App& app)
	: Game(app), m_game_view(app.get_render_device().render_view), m_score_text(&app.get_render_device()), m_game_over_text(&app.get_render_device()),
	  m_restart_text(&app.get_render_device()) {
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_width(m_config.world_space);
	m_config.world_space = m_game_view.viewport;

	auto loader = Loader{&app.get_data_store(), &app.get_render_device()};

	if (auto font = loader.load_font("fonts/Vera.ttf")) {
		m_score_text.set_font(font);
		m_game_over_text.set_font(font);
		m_restart_text.set_font(font);

		m_score_text.tint = m_config.score_text_rgba;
		m_game_over_text.tint = m_config.game_over_text_rgba;
		m_restart_text.tint = m_config.restart_text_rgba;
	}

	m_game_view.viewport = app.get_render_device().get_viewport_scaler().match_width(m_config.world_space);
	m_config.world_space = m_game_view.viewport;

	create_entities();

	m_score_text.set_string("0");
	m_score_text.transform.position.y = m_config.score_text_y;

	m_restart_text.transform.position.y = m_config.restart_text_y;
	m_restart_text.set_height(m_config.restart_text_height);
}

void Flappy::tick() {
	auto const dt = get_app().get_dt();

	if (!m_paused) {
		if (m_pipes->pipe_exists()) { m_score_elapsed += dt; }
		m_pipes->tick(dt);
		m_player->tick(dt);

		auto const hitbox = Rect<>::from_extent(m_config.player_hitbox, m_player->sprite.transform.position);
		if (m_pipes->is_colliding(hitbox)) { game_over(); }

		if (std::abs(m_player->sprite.transform.position.y) > 0.5f * m_config.world_space.y) { game_over(); }
	}

	m_background->tick(dt);

	if (m_exploding) {
		m_explode->tick(dt);
		m_exploding = m_explode->animate;
		m_log.debug("exploding");
	}

	m_score_text.set_string(fmt::format("{:.0f}", m_score_elapsed.count()));

	if (m_game_over) {
		m_game_over_text.set_string("GAME OVER");
		m_game_over_elapsed += dt;
		if (m_game_over_elapsed >= 2s) {
			m_restart_text.set_string("tap to restart");
			if (get_app().get_gesture_recognizer().tap_up()) { restart(); }
		}
	}

	if (m_force_lag) { std::this_thread::sleep_for(30ms); }

	IFBAVEIMGUI({
		ImGui::ShowDemoWindow();
		if (ImGui::Begin("Debug")) {
			auto const* pause_text = m_paused ? "Play" : "Pause";
			if (ImGui::Button(pause_text)) { m_paused = !m_paused; }

			ImGui::Checkbox("force lag", &m_force_lag);
		}
		ImGui::End();
	});
}

void Flappy::render() const {
	get_app().get_render_device().render_view = m_game_view;
	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) {
		m_background->draw(*shader);
		m_pipes->draw(*shader);
		if (!m_game_over) { m_player->draw(*shader); }
		if (m_exploding) { m_explode->draw(*shader); }
		m_score_text.draw(*shader);
		m_game_over_text.draw(*shader);
		m_restart_text.draw(*shader);
	}
}

void Flappy::on_key(KeyInput const& key_input) {
	if (key_input.key == Key::eW && key_input.mods.test(bave::mod::ctrl)) {
		m_log.info("shutting down");
		get_app().shutdown();
	}

	if (key_input.key == Key::eEscape && key_input.action == Action::eRelease) {
		m_log.info("shutting down");
		get_app().shutdown();
	}
	if (key_input.key == Key::eSpace) {
		if (key_input.action == Action::ePress) {
			m_player->start_jump();
		} else {
			m_player->stop_jump();
		}
	}
}

void Flappy::on_tap(PointerTap const& tap) {
	if (tap.action == Action::ePress) {
		m_player->start_jump();
	} else {
		m_player->stop_jump();
	}
}

void Flappy::on_focus(bave::FocusChange const& focus) {
	if (m_game_over) { return; }
	m_paused = !focus.in_focus;
}

void Flappy::create_entities() {
	auto& render_device = get_app().get_render_device();
	auto loader = Loader{&get_app().get_data_store(), &render_device};

	auto explode_sheet = loader.load_sprite_sheet("images/explode_sheet.json");
	auto explode_anim = loader.load_sprite_animation("animations/explode_anim.json");

	m_player = Player{&render_device, &m_config};
	m_background = Background{&render_device, &m_config};
	m_pipes = Pipes{&render_device, &m_config};
	m_explode = AnimatedSprite{&render_device, std::move(explode_sheet)};
	if (explode_anim) { m_explode->animation = *explode_anim; }
	m_explode->repeat = false;

	if (auto texture = loader.load_texture("images/bird_256x256.png")) {
		texture->sampler.min = texture->sampler.mag = Texture::Filter::eNearest;
		m_player->sprite.set_texture(std::move(texture));
	}
	m_background->cloud.set_texture(loader.load_texture(m_config.cloud_texture));
	m_pipes->pipe_texture = loader.load_texture(m_config.pipe_texture);
}

void Flappy::game_over() {
	m_score_text.set_string("game over");
	m_game_over = m_paused = true;
	m_explode->transform.position = m_player->sprite.transform.position;
	m_explode->animate = m_exploding = true;
}

void Flappy::restart() {
	m_restart_text.set_string({});
	m_game_over_text.set_string({});
	m_game_over_elapsed = {};
	m_score_elapsed = {};
	m_player->restart();
	m_pipes->restart();
	m_game_over = m_paused = false;
}
