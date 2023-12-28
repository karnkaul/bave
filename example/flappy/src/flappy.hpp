#pragma once
#include <bave/game.hpp>
#include <bave/graphics/sprite.hpp>
#include <bave/graphics/text.hpp>
#include <src/background.hpp>
#include <src/pipes.hpp>
#include <src/player.hpp>
#include <future>

class Flappy : public bave::Game {
	void tick() final;
	void render() const final;

	void on_key(bave::KeyInput const& key_input) final;
	void on_tap(bave::PointerTap const& tap) final;
	void on_focus(bave::FocusChange const& focus) final;

	void setup_viewport();
	void load_assets();
	void create_entities();
	void setup_hud();

	void poll_futures();
	void game_over();
	void restart();

	bave::Logger m_log{"Flappy"};

	bave::RenderView m_game_view{};

	Config m_config{};
	std::future<std::shared_ptr<bave::AudioClip>> m_music_future{};

	std::optional<Player> m_player{};
	std::optional<Background> m_background{};
	std::optional<Pipes> m_pipes{};
	std::optional<bave::AnimatedSprite> m_explode{};

	bave::Text m_score_text;
	bave::Text m_game_over_text;
	bave::Text m_restart_text;

	bave::Seconds m_score_elapsed{};
	bool m_paused{};
	bool m_game_over{};
	bave::Seconds m_game_over_elapsed{};
	bool m_exploding{};

	bool m_force_lag{};

  public:
	explicit Flappy(bave::App& app);
};
