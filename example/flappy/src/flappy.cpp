#include <bave/graphics/pixmap.hpp>
#include <bave/graphics/projector.hpp>
#include <bave/json_io.hpp>
#include <bave/loader.hpp>
#include <src/flappy.hpp>

namespace {
void load_config(Config& out, bave::DataStore const& data_store, std::string_view uri = "config.json") {
	using bave::from_json;
	if (auto const json = data_store.read_json(uri)) {
		if (json.contains("world_space")) { from_json(json["world_space"], out.world_space); }

		if (json.contains("player_size")) { from_json(json["player_size"], out.player_size); }
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
	}
}
} // namespace

Flappy::Flappy(bave::App& app) : Game(app), m_game_view(app.get_render_device().render_view), m_text(&app.get_render_device()) {
	auto loader = bave::Loader{&app.get_data_store(), &app.get_render_device()};

	if (auto font = loader.load_font("fonts/Vera.ttf")) { m_text.set_font(std::move(font)); }

	load_config(m_config, get_app().get_data_store());
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_height(m_config.world_space);
	m_config.world_space = m_game_view.viewport;
	m_player = Player{app, &m_config};
	m_background = Background{&app.get_render_device(), &m_config};
	assign_assets();

	m_text.set_string("hello");
	// m_text.set_align(bave::Text::Align::eLeft);
	m_text.transform.position.y = 500.0f;
}

void Flappy::tick() {
	auto const dt = get_app().get_dt();

	m_background->tick(dt);
	if (!m_paused) { m_player->tick(dt); }

	IFBAVEIMGUI({
		ImGui::ShowDemoWindow();
		if (ImGui::Begin("Debug")) {
			if (ImGui::Button("Reload config")) { reload_config(); }

			ImGui::Separator();
			auto const* pause_text = m_paused ? "Play" : "Pause";
			if (ImGui::Button(pause_text)) { m_paused = !m_paused; }
		}
		ImGui::End();
	});
}

void Flappy::render() const {
	get_app().get_render_device().render_view = m_game_view;
	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) {
		m_background->draw(*shader);
		m_player->draw(*shader);
		m_text.draw(*shader);
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
}

void Flappy::reload_config() {
	load_config(m_config, get_app().get_data_store());
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_height(m_config.world_space);
	m_config.world_space = m_game_view.viewport;
	assign_assets();
}
