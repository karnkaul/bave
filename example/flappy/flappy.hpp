#pragma once
#include <bave/game.hpp>

class Flappy : public bave::Game {
	void tick() final;
	[[nodiscard]] auto get_clear() const -> bave::Rgba final { return bave::Rgba::from({m_clear_red, 0.0f, 0.0f, 1.0f}); }

	bave::Logger m_log{"Flappy"};
	bave::Seconds m_elapsed{};

	float m_clear_red{};

  public:
	using bave::Game::Game;
};
