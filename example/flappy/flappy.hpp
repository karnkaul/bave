#pragma once
#include <bave/game.hpp>
#include <bave/graphics/sprite.hpp>
#include <bave/graphics/text.hpp>

class Flappy : public bave::Game {
	void tick() final;
	void render() const final;

	void on_key(bave::KeyInput key_input) final;
	void on_move(bave::PointerMove pointer_move) final;
	void on_scroll(bave::MouseScroll mouse_scroll) final;

	bave::Logger m_log{"Flappy"};
	bave::Seconds m_elapsed{};

	bave::RenderView m_game_view{};

	bave::Text m_text;

	std::shared_ptr<bave::SpriteSheet> m_sheet{};
	std::shared_ptr<bave::SpriteAnimation> m_animation{};
	std::optional<bave::AnimatedSprite> m_sprite{};
	glm::vec2 m_target{};

	float m_clear_red{};
	glm::vec2 m_prev_pointer{};
	glm::vec2 m_pointer{};

  public:
	explicit Flappy(bave::App& app);
};
