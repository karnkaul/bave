#pragma once
#include <bave/core/is_positive.hpp>

namespace bave {
/// \brief Projects a point from source space to target space.
struct Projector {
	/// \brief Source space.
	glm::vec2 source{};
	/// \brief Target space.
	glm::vec2 target{};

	/// \brief Project a point.
	/// \param point Point in source space.
	/// \returns Point in target space.
	[[nodiscard]] constexpr auto project(glm::vec2 const point) const -> glm::vec2 {
		if (!is_positive(source) || source == target) { return point; }
		return point * target / source;
	}

	[[nodiscard]] constexpr auto operator()(glm::vec2 const point) const -> glm::vec2 { return project(point); }
};
} // namespace bave
