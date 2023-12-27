#pragma once
#include <bave/game.hpp>
#include <bave/graphics/sprite.hpp>
#include <bave/graphics/text.hpp>
#include <src/background.hpp>
#include <src/player.hpp>

class Flappy : public bave::Game {
	void tick() final;
	void render() const final;

	void on_key(bave::KeyInput const& key_input) final;
	void on_tap(bave::PointerTap const& tap) final;

	void assign_assets();
	void reload_config();

	bave::Logger m_log{"Flappy"};

	bave::RenderView m_game_view{};

	Config m_config{};

	std::optional<Player> m_player{};
	std::optional<Background> m_background{};

	bave::Text m_text;

	bool m_paused{};

  public:
	explicit Flappy(bave::App& app);
};
