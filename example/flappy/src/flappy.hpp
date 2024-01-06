#pragma once
#include <bave/game.hpp>
#include <bave/graphics/sprite.hpp>
#include <bave/graphics/text.hpp>
#include <bave/im_texture.hpp>
#include <src/background.hpp>
#include <src/pipes.hpp>
#include <src/player.hpp>
#include <future>

// entry point Game subclass.
// the entry point is setup in bave via a 'game factory' callback in bave::App.  (see main.cpp)
class Flappy : public bave::Game {
	// tick is called every frame, until the engine is shutting down.
	void tick() final;
	// render is called after tick, unless there is no render target to draw to (eg the Vulkan Swapchain got resized).
	void render() const final;

	// optional input callbacks.
	// on_key is called on a KeyInput event.
	void on_key(bave::KeyInput const& key_input) final;
	// on_tap is called on a PointerTap (touch or mouse click) event.
	// on desktop there is only one pointer (the mouse), but on Android there may be multiple active simultaneously,
	// so checking tap.pointer.id is recommended.
	void on_tap(bave::PointerTap const& tap) final;
	// on_focus is called on the window changing focus.
	void on_focus(bave::FocusChange const& focus) final;

	void setup_viewport();
	void load_assets();
	void create_entities();
	void setup_hud();

	void poll_futures();
	void game_over();
	void restart();

	void interact_start();
	void interact_stop();

	[[nodiscard]] auto is_active() const -> bool { return m_pipes->pipe_exists(); }
	[[nodiscard]] auto can_restart() const -> bool { return m_game_over_elapsed > m_config.restart_delay; }

	bave::Logger m_log{"Flappy"};

	bave::RenderView m_game_view{};

	Config m_config{};
	std::future<std::shared_ptr<bave::AudioClip>> m_music_future{};

	std::optional<Player> m_player{};
	std::optional<Background> m_background{};
	std::optional<Pipes> m_pipes{};
	std::optional<bave::AnimatedSprite> m_explode{};

	bave::QuadShape m_score_bg;
	bave::Text m_score_text;
	bave::Text m_game_over_text;
	bave::Text m_restart_text;

	int m_score{};
	bool m_paused{};
	bool m_game_over{};
	bave::Seconds m_game_over_elapsed{};
	bool m_exploding{};

	bool m_force_lag{};

	std::optional<bave::ImTexture> m_im_texture{};

  public:
	// constructor needs to be public (or at least accessible by the game factory that's setup in the main target)
	explicit Flappy(bave::App& app);
};
