#include <backends/imgui_impl_vulkan.h>
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

// bave will reset delta time after using game factory, so time spent in this constructor will not bloat up the first tick's dt.
// it will still halt the app until complete though; taking too long might trigger an ANR (App Not Responding) on Android.
Flappy::Flappy(App& app)
	: Game(app), m_game_view(app.get_render_device().render_view), m_score_bg(&app.get_render_device()), m_score_text(&app.get_render_device()),
	  m_game_over_text(&app.get_render_device()), m_restart_text(&app.get_render_device()) {
	// we use a custom / fixed viewport so that the same game world is visible regardless of screen / framebuffer size.
	setup_viewport();
	// this example loads most assets on the main thread, but all of them can be loaded asynchronously if desired.
	load_assets();
	// create and setup all the game entities.
	create_entities();
	// create and setup the HUD (mainly text here).
	setup_hud();

	m_im_texture = bave::ImTexture{m_config.cloud_texture};
}

void Flappy::tick() {
	// check async asset load status, and start using if ready.
	poll_futures();

	auto const dt = get_app().get_dt();

	if (!m_paused) {
		// only tick the player if the game is active, ie, user is in control of the player.
		if (is_active()) { m_player->tick(dt); }
		// Pipes::tick returns true if a pipe crosses the player, increment the score accordingly.
		if (m_pipes->tick(dt)) { ++m_score; }

		// check for player collision with pipes.
		auto const hitbox = Rect<>::from_extent(m_config.player_hitbox, m_player->sprite.transform.position);
		if (m_pipes->is_colliding(hitbox)) { game_over(); }

		// also check for player crossing the top/bottom edges of the game world / screen.
		if (std::abs(m_player->sprite.transform.position.y) > 0.5f * m_config.world_space.y) { game_over(); }
	}

	// background is always ticked.
	m_background->tick(dt);

	if (m_exploding) {
		// update animation.
		m_explode->tick(dt);
		// stop updating if animation has completed one cycle.
		m_exploding = m_explode->animate;
	}

	// update score.
	m_score_text.set_string(fmt::format("{}", m_score));

	// update buffer time after death before respawn is enabled.
	if (m_game_over) { m_game_over_elapsed += dt; }

	// debug stuff.
	if (m_force_lag) { std::this_thread::sleep_for(30ms); }

	// ImGui is not available if bave::imgui_v is false.
	// its headers (and thus declarations) are available regardless, enabling usage of such if constexpr blocks,
	// instead of having to resort to using macros.
	// in short, if this code is not stripped out at compile time, linking will fail on Android due to undefined symbols.
	if constexpr (bave::imgui_v) {
		ImGui::ShowDemoWindow();
		if (ImGui::Begin("Debug")) {
			auto const* pause_text = m_paused ? "Play" : "Pause";
			if (ImGui::Button(pause_text)) { m_paused = !m_paused; }

			ImGui::Checkbox("force lag", &m_force_lag);

			if (m_im_texture->get_id() != vk::DescriptorSet{}) {
				ImGui::Separator();
				glm::vec2 const size = m_im_texture->get_texture()->get_size();
				ImGui::Image(m_im_texture->get_id(), {size.x, size.y});
			}
		}
		ImGui::End();
	}
}

void Flappy::render() const {
	// make sure we're using the correct view for rendering.
	// it's possible to change the view at any time before a draw, eg use a different one for game vs UI.
	// this example doesn't need that, as it uses a fixed view whose transform doesn't change during gameplay.
	get_app().get_render_device().render_view = m_game_view;
	// load default shader for drawing.
	// a single Shader instance can be used for multiple draws.
	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) {
		// always draw background and pipes.
		m_background->draw(*shader);
		m_pipes->draw(*shader);

		// skip drawing player if dead.
		if (!m_game_over) { m_player->draw(*shader); }

		// skip drawing explode animation if it's not animating.
		if (m_exploding) { m_explode->draw(*shader); }

		// always draw score.
		m_score_bg.draw(*shader);
		m_score_text.draw(*shader);

		if (m_game_over) {
			// draw 'GAME OVER' if dead.
			m_game_over_text.draw(*shader);
			// draw 'press tap to restart' if respawn is enabled.
			if (can_restart()) { m_restart_text.draw(*shader); }
		}
	}
}

// key inputs will not occur on Android unless an external keyboard is connected (or keyboard emulation is enabled on an Android Virtual Device).
void Flappy::on_key(KeyInput const& key_input) {
	// Esc shuts down the game.
	if (key_input.key == Key::eEscape && key_input.action == Action::eRelease) {
		m_log.info("shutting down");
		get_app().shutdown();
	}
	// Space emulates taps.
	if (key_input.key == Key::eSpace) {
		if (key_input.action == Action::ePress) {
			interact_start();
		} else {
			interact_stop();
		}
	}
}

// taps encompass both touches and mouse clicks.
void Flappy::on_tap(PointerTap const& tap) {
	// the first touch will always have its ID == ePrimary. subsequent touches (in the same gesture) will have monotonically increasing IDs.
	// on desktop there's only one pointer which is always ePrimary.
	// we only deal with primary taps here.
	if (tap.pointer.id != PointerId::ePrimary) { return; }
	if (tap.action == Action::ePress) {
		interact_start();
	} else {
		interact_stop();
	}
}

void Flappy::on_focus(FocusChange const& focus) {
	// don't accidentally unpause if dead.
	if (m_game_over) { return; }
	// pause/unpause gameplay based on whether the window/app has focus.
	m_paused = !focus.in_focus;
}

void Flappy::setup_viewport() {
	// setup the viewport to a fixed width, with its height scaled such that the native aspect ratio remains constant.
	// this means that the width of the game view will be identical on all screen sizes / aspect ratios, with the height varying.
	m_game_view.viewport = get_app().get_render_device().get_viewport_scaler().match_width(m_config.world_space);
	// update the actual world space to the scaled viewport.
	m_config.world_space = m_game_view.viewport;
}

void Flappy::load_assets() {
	auto const loader = Loader{&get_app().get_data_store(), &get_app().get_render_device()};
	// setup an async load.
	m_music_future = std::async([loader] { return loader.load_audio_clip("audio_clips/in_the_city.mp3"); });

	// load the rest before returning.
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

	// explode animation.
	m_explode = AnimatedSprite{&render_device, m_config.explode_sheet};
	if (m_config.explode_animation) { m_explode->animation = *m_config.explode_animation; }
	m_explode->repeat = false;

	// player sprite.
	m_player = Player{&get_app(), &m_config};
	// background (sky gradient, scrolling clouds).
	m_background = Background{&render_device, &m_config};
	// pipes.
	m_pipes = Pipes{&render_device, &m_config};
}

void Flappy::setup_hud() {
	// set fonts.
	m_score_text.set_font(m_config.hud_font);
	m_game_over_text.set_font(m_config.hud_font);
	m_restart_text.set_font(m_config.hud_font);

	// set tints.
	m_score_text.tint = m_config.score_text_rgba;
	m_game_over_text.tint = m_config.game_over_text_rgba;
	m_restart_text.tint = m_config.restart_text_rgba;

	// setup layout.
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
	// check if async asset has loaded.
	if (m_music_future.valid() && m_music_future.wait_for(0s) == std::future_status::ready) {
		// obtain its value if so.
		m_config.music = m_music_future.get();
		// and start playing.
		// App will automatically pause/resume this AudioStreamer instance's playback on focus change.
		// if using a custom AudioStreamer instance, that will have to be handled manually.
		if (m_config.music) { get_app().get_audio_streamer().play(m_config.music); }
	}
}

void Flappy::game_over() {
	// pause gameplay.
	m_game_over = m_paused = true;
	// move explode animation to where player died.
	m_explode->transform.position = m_player->sprite.transform.position;
	// start animating.
	m_explode->animate = m_exploding = true;
	m_explode->elapsed = {};
	// play SFX.
	if (m_config.explode_sfx) { get_app().get_audio_device().play_once(*m_config.explode_sfx); }
}

void Flappy::restart() {
	// reset all gameplay state.
	m_game_over_elapsed = {};
	m_score = {};
	m_player->restart();
	m_pipes->restart();
	m_game_over = m_paused = false;
}

void Flappy::interact_start() {
	if (m_game_over) {
		// tap / Space => restart.
		if (can_restart()) { restart(); }
	} else {
		// tap / Space => start jumping.
		if (is_active()) { m_player->start_jump(); }
	}
}

void Flappy::interact_stop() {
	// tap => stop jumping.
	if (!m_game_over) { m_player->stop_jump(); }
}
