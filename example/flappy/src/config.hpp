#pragma once
#include <bave/audio/audio_clip.hpp>
#include <bave/core/time.hpp>
#include <bave/font/font.hpp>
#include <bave/font/text_height.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/sprite_animation.hpp>
#include <bave/graphics/texture_atlas.hpp>
#include <glm/vec2.hpp>
#include <memory>

struct Config {
	glm::vec2 world_space{720.0f, 1280.0f};

	glm::vec2 player_size{100.0f};
	glm::vec2 player_hitbox{78.0f, 56.0f};
	float gravity{700.0f};
	float jump_speed{1000.0f};
	bave::Seconds max_jump_duration{1s};
	std::shared_ptr<bave::Texture> player_texture{};
	std::shared_ptr<bave::AudioClip> jump_sfx{};

	std::shared_ptr<bave::TextureAtlas> explode_atlas{};
	std::optional<bave::SpriteAnimation> explode_animation{};
	std::shared_ptr<bave::AudioClip> explode_sfx{};

	bave::Rgba background_rgba_top{bave::Rgba::from(0x1c96c5ff)};
	bave::Rgba background_rgba_bottom{bave::Rgba::from(0xa0d9efff)};
	glm::vec2 cloud_size{200.0f, 100.0f};
	int cloud_instances{10};
	float cloud_speed_min{50.0f};
	float cloud_speed_max{80.0f};
	std::shared_ptr<bave::Texture> cloud_texture{};

	glm::vec2 pipe_size{150.0f, 1200.0f};
	float pipe_speed{400.0f};
	float pipe_gap{200.0f};
	bave::Seconds pipe_period{1.4s};
	std::shared_ptr<bave::Texture> pipe_texture{};

	float score_text_y{500.0f};
	bave::Rgba score_text_rgba{bave::Rgba::from(0xffffffff)};
	bave::TextHeight score_text_height{bave::TextHeight{80}};
	bave::Rgba score_bg_rgba{bave::Rgba::from(0x000000cc)};
	bave::Rgba game_over_text_rgba{bave::Rgba::from(0xcc4400ff)};
	bave::TextHeight game_over_text_height{bave::TextHeight{80}};
	float restart_text_y{-200.0f};
	bave::Rgba restart_text_rgba{bave::black_v};
	bave::TextHeight restart_text_height{bave::TextHeight{40}};

	std::shared_ptr<bave::Font> hud_font{};

	std::shared_ptr<bave::AudioClip> music{};

	bave::Seconds restart_delay{1s};
};
