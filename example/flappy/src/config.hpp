#pragma once
#include <bave/core/time.hpp>
#include <bave/graphics/rgba.hpp>
#include <glm/vec2.hpp>
#include <string>

struct Config {
	glm::vec2 world_space{720.0f, 1280.0f}; // NOLINT

	glm::vec2 player_size{100.0f};		   // NOLINT
	glm::vec2 player_hitbox{80.0f};		   // NOLINT
	float gravity{-500.0f};				   // NOLINT
	float jump_speed{1000.0f};			   // NOLINT
	bave::Seconds max_jump_duration{0.5s}; // NOLINT
	std::string player_texture{};

	bave::Rgba background_rgba_top{bave::blue_v};
	bave::Rgba background_rgba_bottom{bave::white_v};
	glm::vec2 cloud_size{200.0f, 100.0f}; // NOLINT
	int cloud_instances{10};			  // NOLINT
	float cloud_speed_min{-100.0f};		  // NOLINT
	float cloud_speed_max{-300.0f};		  // NOLINT
	std::string cloud_texture{};

	glm::vec2 pipe_size{50.0f, 200.0f}; // NOLINT
	float pipe_speed{-500.0f};			// NOLINT
	float pipe_gap{300.0f};				// NOLINT
	bave::Seconds pipe_period{1s};
	std::string pipe_texture{};
};
