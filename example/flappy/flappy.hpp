#pragma once
#include <bave/game.hpp>

class Flappy : public bave::Game {
	void tick() final;

	bave::Logger m_log{"Flappy"};
	bave::Seconds m_elapsed{};

	glm::ivec2 m_fb_size{};

  public:
	using bave::Game::Game;
};
