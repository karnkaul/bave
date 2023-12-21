#pragma once
#include <bave/core/is_positive.hpp>

namespace bave {
///
/// \brief Projects a point from source space to target space.
///
struct Projector {
	glm::vec2 source{};
	glm::vec2 target{};

	[[nodiscard]] constexpr auto project(glm::vec2 const point) const -> glm::vec2 {
		if (!is_positive(source) || source == target) { return point; }
		return point * target / source;
	}

	[[nodiscard]] constexpr auto operator()(glm::vec2 const point) const -> glm::vec2 { return project(point); }
};
} // namespace bave
