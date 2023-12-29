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
using bave::PointerId;
using bave::PointerTap;
using bave::Rect;
using bave::Seconds;
using bave::Texture;

Flappy::Flappy(App& app)
	: Game(app), m_game_view(app.get_render_device().render_view), m_score_bg(&app.get_render_device()), m_score_text(&app.get_render_device()),
	  m_game_over_text(&app.get_render_device()), m_restart_text(&app.get_render_device()) {
	setup_viewport();
	load_assets();
	create_entities();
	setup_hud();
}

void Flappy::tick() {
	poll_futures();

	auto const dt = get_app().get_dt();

	if (!m_paused) {
		if (is_active()) { m_player->tick(dt); }
		if (m_pipes->tick(dt)) { ++m_score; }

		auto const hitbox = Rect<>::from_extent(m_config.player_hitbox, m_player->sprite.transform.position);
		if (m_pipes->is_colliding(hitbox)) { game_over(); }

		if (std::abs(m_player->sprite.transform.position.y) > 0.5f * m_config.world_space.y) { game_over(); }
	}

	m_background->tick(dt);

	if (m_exploding) {
		m_explode->tick(dt);
		m_exploding = m_explode->animate;
	}

	m_score_text.set_string(fmt::format("{}", m_score));

	if (m_game_over) { m_game_over_elapsed += dt; }

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

		m_score_bg.draw(*shader);
		m_score_text.draw(*shader);

		if (m_game_over) {
			m_game_over_text.draw(*shader);
			if (can_restart()) { m_restart_text.draw(*shader); }
		}
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
			interact_start();
		} else {
			interact_stop();
		}
	}
}

void Flappy::on_tap(PointerTap const& tap) {
	if (tap.pointer.id != PointerId::ePrimary) { return; }
	if (tap.action == Action::ePress) {
		interact_start();
	} else {
		interact_stop();
	}
}

void Flappy::on_focus(FocusChange const& focus) {
	if (m_game_over) { return; }
	m_paused = !focus.in_focus;
}

void Flappy::setup_viewport() {
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_width(m_config.world_space);
	m_config.world_space = m_game_view.viewport;
}

void Flappy::load_assets() {
	auto const loader = Loader{&get_app().get_data_store(), &get_app().get_render_device()};
	m_music_future = std::async([loader] { return loader.load_audio_clip("audio_clips/in_the_city.mp3"); });

	m_config.player_texture = loader.load_texture("images/bird_256x256.png");
	m_config.jump_sfx = loader.load_audio_clip("audio_clips/beep.wav");

	m_config.explode_sheet = loader.load_sprite_sheet("images/explode_sheet.json");
	m_config.explode_animation = loader.load_sprite_animation("animations/explode_anim.json");
	m_config.explode_sfx = loader.load_audio_clip("audio_clips/explode.wav");

	m_config.cloud_texture = loader.load_texture("images/cloud_256x128.png");
	m_config.pipe_texture = loader.load_texture("images/pipe_128x1024.png");
	m_config.hud_font = loader.load_font("fonts/Vera.ttf");

	if (m_config.player_texture) { m_config.player_texture->sampler.min = m_config.player_texture->sampler.mag = Texture::Filter::eNearest; }
}

void Flappy::create_entities() {
	auto& render_device = get_app().get_render_device();

	m_explode = AnimatedSprite{&render_device, m_config.explode_sheet};
	if (m_config.explode_animation) { m_explode->animation = *m_config.explode_animation; }
	m_explode->repeat = false;

	m_player = Player{&get_app(), &m_config};
	m_background = Background{&render_device, &m_config};
	m_pipes = Pipes{&render_device, &m_config};
}

void Flappy::setup_hud() {
	m_score_text.set_font(m_config.hud_font);
	m_game_over_text.set_font(m_config.hud_font);
	m_restart_text.set_font(m_config.hud_font);

	m_score_text.tint = m_config.score_text_rgba;
	m_game_over_text.tint = m_config.game_over_text_rgba;
	m_restart_text.tint = m_config.restart_text_rgba;

	m_score_text.set_string("0");
	m_score_text.transform.position.y = m_config.score_text_y;
	m_score_text.set_height(m_config.score_text_height);

	auto const score_bounds = m_score_text.get_bounds();
	m_score_bg.set_shape(bave::Quad{.size = {m_config.world_space.x, 1.5f * score_bounds.extent().y}});
	m_score_bg.tint = m_config.score_bg_rgba;
	m_score_bg.transform.position = score_bounds.centre();

	m_game_over_text.set_string("GAME OVER");
	m_game_over_text.set_height(m_config.game_over_text_height);

	m_restart_text.set_string("tap to restart");
	m_restart_text.transform.position.y = m_config.restart_text_y;
	m_restart_text.set_height(m_config.restart_text_height);
}

void Flappy::poll_futures() {
	if (m_music_future.valid() && m_music_future.wait_for(0s) == std::future_status::ready) {
		m_config.music = m_music_future.get();
		if (m_config.music) { get_app().get_audio_streamer().play(m_config.music); }
	}
}

void Flappy::game_over() {
	m_game_over = m_paused = true;
	m_explode->transform.position = m_player->sprite.transform.position;
	m_explode->animate = m_exploding = true;
	if (m_config.explode_sfx) { get_app().get_audio_device().play_once(*m_config.explode_sfx); }
}

void Flappy::restart() {
	m_game_over_elapsed = {};
	m_score = {};
	m_player->restart();
	m_pipes->restart();
	m_game_over = m_paused = false;
}

void Flappy::interact_start() {
	if (m_game_over) {
		if (can_restart()) { restart(); }
	} else {
		if (is_active()) { m_player->start_jump(); }
	}
}

void Flappy::interact_stop() {
	if (!m_game_over) { m_player->stop_jump(); }
}
