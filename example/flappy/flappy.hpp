#pragma once
#include <bave/game.hpp>
#include <bave/shape.hpp>

class Flappy : public bave::Game {
	void tick() final;
	void render(vk::CommandBuffer command_buffer) const final;

	bave::Logger m_log{"Flappy"};
	bave::Seconds m_elapsed{};

	bave::QuadShape m_quad;

	float m_clear_red{};

  public:
	explicit Flappy(bave::App& app);
};
