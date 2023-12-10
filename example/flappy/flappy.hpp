#pragma once
#include <bave/game.hpp>
#include <bave/graphics/mesh.hpp>
#include <bave/graphics/texture.hpp>

class Flappy : public bave::Game {
	void tick() final;
	void render(vk::CommandBuffer command_buffer) const final;

	bave::Logger m_log{"Flappy"};
	bave::Seconds m_elapsed{};

	bave::Mesh m_mesh;
	bave::Texture m_texture;

	float m_clear_red{};

  public:
	explicit Flappy(bave::App& app);
};
