#pragma once
#include <bave/core/radians.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace bave {
struct Transform {
	glm::vec2 position{};
	Radians rotation{};
	glm::vec2 scale{1.0f};

	[[nodiscard]] auto matrix() const -> glm::mat4;
};

static_assert(std::is_trivially_copyable_v<Transform>);
} // namespace bave
