#pragma once
#include <bave/graphics/rect.hpp>

namespace bave {
/// \brief A single character glyph.
struct Glyph {
	glm::vec2 advance{};
	glm::vec2 extent{};
	glm::vec2 left_top{};
	UvRect uv_rect{};

	[[nodiscard]] constexpr auto rect(glm::vec2 baseline, float const scale = 1.0f) const -> Rect<> {
		return {.lt = baseline + scale * left_top, .rb = baseline + scale * (left_top + glm::vec2{extent.x, -extent.y})};
	}

	constexpr void scale(float const scale) {
		advance *= scale;
		extent *= scale;
		left_top *= scale;
	}

	explicit constexpr operator bool() const { return advance.x > 0 || advance.y > 0; }
};
} // namespace bave
