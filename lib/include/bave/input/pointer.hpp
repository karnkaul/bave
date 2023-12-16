#pragma once
#include <bave/input/action.hpp>
#include <glm/vec2.hpp>

namespace bave {

struct Pointer {
	enum Id : int { ePrimary = 0 };

	Id id{};
	glm::vec2 position{};
};
} // namespace bave
