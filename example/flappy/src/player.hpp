#pragma once
#include <bave/graphics/sprite.hpp>
#include <src/config.hpp>

class Player {
  public:
	Player(bave::NotNull<bave::RenderDevice*> render_device, bave::NotNull<Config const*> config);

	void tick(bave::Seconds dt);
	void draw(bave::Shader& shader) const;

	void start_jump();
	void stop_jump();
	void restart();

	bave::NotNull<Config const*> config;

	bave::Sprite sprite;

  private:
	std::optional<bave::Seconds> m_jump_elapsed{};
};
