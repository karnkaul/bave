#pragma once
#include <bave/input/action.hpp>
#include <glm/vec2.hpp>

namespace bave {
enum class PointerId : int { ePrimary = 0 };

struct Pointer {
	using Id = PointerId;

	Id id{};
	glm::vec2 position{};
};
} // namespace bave
