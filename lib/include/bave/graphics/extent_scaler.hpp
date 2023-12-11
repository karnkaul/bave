#pragma once
#include <glm/vec2.hpp>

namespace bave {
///
/// \brief Maintains source aspect ratio and scales positive extents.
///
struct ExtentScaler {
	glm::vec2 source{};

	[[nodiscard]] static constexpr auto is_positive(glm::vec2 const extent) -> bool { return extent.x > 0.0f && extent.y > 0.0f; }

	[[nodiscard]] static constexpr auto aspect_ratio(glm::vec2 const extent) -> float {
		if (!is_positive(extent)) { return {}; }
		return extent.x / extent.y;
	}

	[[nodiscard]] constexpr auto aspect_ratio() const -> float { return aspect_ratio(source); }

	[[nodiscard]] constexpr auto match_width(glm::vec2 const of) const -> glm::vec2 {
		if (!is_positive(source) || !is_positive(of)) { return {}; }
		auto const scale = of.x / source.x;
		return scale * source;
	}

	[[nodiscard]] constexpr auto match_height(glm::vec2 const of) const -> glm::vec2 {
		if (!is_positive(source) || !is_positive(of)) { return {}; }
		auto const scale = of.y / source.y;
		return scale * source;
	}

	[[nodiscard]] constexpr auto match_smaller(glm::vec2 const of) const -> glm::vec2 {
		if (of.x < source.x) { return match_width(of); }
		return match_height(of);
	}

	[[nodiscard]] constexpr auto match_larger(glm::vec2 const of) const -> glm::vec2 {
		if (of.x > source.x) { return match_width(of); }
		return match_height(of);
	}
};
} // namespace bave
