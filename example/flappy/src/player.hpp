#pragma once
#include <bave/app.hpp>
#include <bave/graphics/sprite.hpp>
#include <src/config.hpp>

class Player {
  public:
	Player(bave::App& app, bave::NotNull<Config const*> config);

	void start_jump();
	void stop_jump();

	void tick(bave::Seconds dt);
	void draw(bave::Shader& shader) const;

	bave::NotNull<Config const*> config;

	bave::Sprite sprite;

  private:
	std::optional<bave::Seconds> m_jump_elapsed{};
};
