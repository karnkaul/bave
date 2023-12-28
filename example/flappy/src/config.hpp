#pragma once
#include <bave/core/time.hpp>
#include <bave/font/text_height.hpp>
#include <bave/graphics/rgba.hpp>
#include <glm/vec2.hpp>
#include <string>

struct Config {
	glm::vec2 world_space{720.0f, 1280.0f}; // NOLINT

	glm::vec2 player_size{100.0f};		   // NOLINT
	glm::vec2 player_hitbox{78.0f, 56.0f}; // NOLINT
	float gravity{-700.0f};				   // NOLINT
	float jump_speed{1000.0f};			   // NOLINT
	bave::Seconds max_jump_duration{1s};   // NOLINT
	std::string player_texture{"images/bird_256x256.png"};

	bave::Rgba background_rgba_top{bave::Rgba::from(0x1c96c5ff)};	 // NOLINT
	bave::Rgba background_rgba_bottom{bave::Rgba::from(0xa0d9efff)}; // NOLINT
	glm::vec2 cloud_size{200.0f, 100.0f};							 // NOLINT
	int cloud_instances{10};										 // NOLINT
	float cloud_speed_min{-50.0f};									 // NOLINT
	float cloud_speed_max{-80.0f};									 // NOLINT
	std::string cloud_texture{"images/cloud_256x128.png"};

	glm::vec2 pipe_size{150.0f, 1200.0f}; // NOLINT
	float pipe_speed{-400.0f};			  // NOLINT
	float pipe_gap{200.0f};				  // NOLINT
	bave::Seconds pipe_period{1.4s};	  // NOLINT
	std::string pipe_texture{"images/pipe_128x1024.png"};

	float score_text_y{500.0f};									  // NOLINT
	bave::Rgba score_text_rgba{bave::Rgba::from(0xcccc22ff)};	  // NOLINT
	bave::Rgba game_over_text_rgba{bave::Rgba::from(0xcc4400ff)}; // NOLINT
	float restart_text_y{-200.0f};								  // NOLINT
	bave::Rgba restart_text_rgba{bave::black_v};				  // NOLINT
	bave::TextHeight restart_text_height{bave::TextHeight{20}};	  // NOLINT

	std::string explode_sheet{"images/explode_sheet.json"};
	std::string explode_anim{"animations/explode_anim.json"};
};
