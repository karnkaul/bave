#pragma once
#include <bave/core/is_positive.hpp>

namespace bave {
/// \brief Maintains source aspect ratio and scales positive extents.
struct ExtentScaler {
	/// \brief Source extent.
	glm::vec2 source{};

	/// \brief Obtain the aspect ratio of a given 2D vector.
	/// \param extent Size of input.
	/// \returns Aspect ratio of input. Zero if extent is non-positive.
	[[nodiscard]] static constexpr auto aspect_ratio(glm::vec2 const extent) -> float {
		if (!is_positive(extent)) { return {}; }
		return extent.x / extent.y;
	}

	/// \brief Obtain the aspect ratio of source.
	/// \returns Aspect ratio of source.
	[[nodiscard]] constexpr auto aspect_ratio() const -> float { return aspect_ratio(source); }

	/// \brief Scale by matching width of input.
	/// \param of Input to match width of.
	/// \returns Scaled extent while maintaining aspect ratio.
	[[nodiscard]] constexpr auto match_width(glm::vec2 const of) const -> glm::vec2 {
		if (!is_positive(source) || !is_positive(of)) { return {}; }
		auto const scale = of.x / source.x;
		return scale * source;
	}

	/// \brief Scale by matching height of input.
	/// \param of Input to match height of.
	/// \returns Scaled extent while maintaining aspect ratio.
	[[nodiscard]] constexpr auto match_height(glm::vec2 const of) const -> glm::vec2 {
		if (!is_positive(source) || !is_positive(of)) { return {}; }
		auto const scale = of.y / source.y;
		return scale * source;
	}

	/// \brief Scale by matching smaller of width and height.
	/// \param of Input to match dimension of.
	/// \returns Scaled extent while maintaining aspect ratio.
	[[nodiscard]] constexpr auto match_smaller(glm::vec2 const of) const -> glm::vec2 {
		if (of.x < source.x) { return match_width(of); }
		return match_height(of);
	}

	/// \brief Scale by matching larger of width and height.
	/// \param of Input to match dimension of.
	/// \returns Scaled extent while maintaining aspect ratio.
	[[nodiscard]] constexpr auto match_larger(glm::vec2 const of) const -> glm::vec2 {
		if (of.x > source.x) { return match_width(of); }
		return match_height(of);
	}

	/// \brief Scale by fitting final rect within given space.
	/// \param in Size limits.
	/// \returns Scaled extent while maintaining aspect ratio.
	[[nodiscard]] constexpr auto fit_space(glm::vec2 const in) const -> glm::vec2 {
		auto const source_ar = aspect_ratio(source);
		auto const dest_ar = aspect_ratio(in);
		if (source_ar > dest_ar) { return match_width(in); }
		return match_height(in);
	}
};
} // namespace bave
