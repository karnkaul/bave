#include <bave/graphics/pixmap.hpp>
#include <bave/graphics/projector.hpp>
#include <bave/json_io.hpp>
#include <bave/loader.hpp>
#include <src/flappy.hpp>
#include <thread>

namespace {
void load_config(Config& out, bave::DataStore const& data_store, std::string_view uri = "config.json") {
	using bave::from_json;
	if (auto const json = data_store.read_json(uri)) {
		if (json.contains("world_space")) { from_json(json["world_space"], out.world_space); }

		if (json.contains("player_size")) { from_json(json["player_size"], out.player_size); }
		if (json.contains("player_hitbox")) { from_json(json["player_hitbox"], out.player_hitbox); }
		out.gravity = json["gravity"].as<float>(out.gravity);
		out.jump_speed = json["jump_speed"].as<float>(out.jump_speed);
		out.max_jump_duration = bave::Seconds{json["max_jump_duration"].as<float>(out.max_jump_duration.count())};
		out.player_texture = json["player_texture"].as_string(out.player_texture);

		if (json.contains("background_rgba_top")) { from_json(json["background_rgba_top"], out.background_rgba_top); }
		if (json.contains("background_rgba_bottom")) { from_json(json["background_rgba_bottom"], out.background_rgba_bottom); }
		if (json.contains("cloud_size")) { from_json(json["cloud_size"], out.cloud_size); }
		out.cloud_instances = json["cloud_instances"].as<int>(out.cloud_instances);
		out.cloud_speed_min = json["cloud_speed_min"].as<float>(out.cloud_speed_min);
		out.cloud_speed_max = json["cloud_speed_max"].as<float>(out.cloud_speed_max);
		out.cloud_texture = json["cloud_texture"].as_string(out.cloud_texture);

		if (json.contains("pipe_size")) { from_json(json["pipe_size"], out.pipe_size); }
		out.pipe_speed = json["pipe_speed"].as<float>(out.pipe_speed);
		out.pipe_gap = json["pipe_gap"].as<float>(out.pipe_gap);
		out.pipe_period = bave::Seconds{json["pipe_period"].as<float>(out.pipe_period.count())};
		out.pipe_texture = json["pipe_texture"].as_string(out.pipe_texture);
	}
}
} // namespace

Flappy::Flappy(bave::App& app)
	: Game(app), m_game_view(app.get_render_device().render_view), m_score_text(&app.get_render_device()), m_game_over_text(&app.get_render_device()),
	  m_restart_text(&app.get_render_device()) {
	auto loader = bave::Loader{&app.get_data_store(), &app.get_render_device()};

	if (auto font = loader.load_font("fonts/Vera.ttf")) {
		m_score_text.set_font(font);
		m_game_over_text.set_font(font);
		m_restart_text.set_font(font);

		m_score_text.tint = bave::Rgba::from(0xcccc22ff);
		m_game_over_text.tint = bave::Rgba::from(0xcc4400ff);
		m_restart_text.tint = bave::black_v;
	}

	load_config(m_config, get_app().get_data_store());
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_width(m_config.world_space);
	m_config.world_space = m_game_view.viewport;
	m_player = Player{app, &m_config};
	m_background = Background{&app.get_render_device(), &m_config};
	m_pipes = Pipes{&app.get_render_device(), &m_config};
	assign_assets();

	m_score_text.set_string("0");
	m_score_text.transform.position.y = 500.0f;

	m_game_over_text.transform.position.y = 0.0f;

	m_restart_text.transform.position.y = -300.0f;
	m_restart_text.set_height(bave::Text::Height{20});
}

void Flappy::tick() {
	auto const dt = get_app().get_dt();

	if (!m_paused) {
		m_score_elapsed += dt;
		m_background->tick(dt);
		m_pipes->tick(dt);
		m_player->tick(dt);

		auto const hitbox = bave::Rect<>::from_extent(m_config.player_hitbox, m_player->sprite.transform.position);
		if (m_pipes->is_colliding(hitbox)) {
			m_score_text.set_string("game over");
			m_game_over = m_paused = true;
		}
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
			if (ImGui::Button("Reload config")) { reload_config(); }

			ImGui::Separator();
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
		m_player->draw(*shader);
		m_score_text.draw(*shader);
		m_game_over_text.draw(*shader);
		m_restart_text.draw(*shader);
	}
}

void Flappy::on_key(bave::KeyInput const& key_input) {
	if (key_input.key == bave::Key::eW && key_input.mods.test(bave::mod::ctrl)) {
		m_log.info("shutting down");
		get_app().shutdown();
	}

	if (key_input.key == bave::Key::eEscape && key_input.action == bave::Action::eRelease) {
		m_log.info("shutting down");
		get_app().shutdown();
	}
	if (key_input.key == bave::Key::eSpace) {
		if (key_input.action == bave::Action::ePress) {
			m_player->start_jump();
		} else {
			m_player->stop_jump();
		}
	}
}

void Flappy::on_tap(bave::PointerTap const& tap) {
	if (tap.action == bave::Action::ePress) {
		m_player->start_jump();
	} else {
		m_player->stop_jump();
	}
}

void Flappy::assign_assets() {
	auto loader = bave::Loader{&get_app().get_data_store(), &get_app().get_render_device()};
	if (auto texture = loader.load_texture(m_config.player_texture)) {
		texture->sampler.min = texture->sampler.mag = bave::Texture::Filter::eNearest;
		m_player->sprite.set_texture(std::move(texture));
	}
	if (auto texture = loader.load_texture(m_config.cloud_texture)) { m_background->cloud.set_texture(std::move(texture)); }
	m_pipes->pipe_texture = loader.load_texture(m_config.pipe_texture);
}

void Flappy::reload_config() {
	load_config(m_config, get_app().get_data_store());
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_width(m_config.world_space);
	m_config.world_space = m_game_view.viewport;
	assign_assets();
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
