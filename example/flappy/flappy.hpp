#pragma once
#include <bave/game.hpp>
#include <bave/graphics/shape.hpp>
#include <bave/input/pinch_detector.hpp>

class Flappy : public bave::Game {
	void tick() final;
	void render(vk::CommandBuffer command_buffer) const final;

	bave::Logger m_log{"Flappy"};
	bave::Seconds m_elapsed{};

	bave::QuadShape m_quad;

	float m_clear_red{};
	bool m_drag{};
	bave::PinchDetector m_pinch{};
	glm::vec2 m_pointer{};

  public:
	explicit Flappy(bave::App& app);
};
