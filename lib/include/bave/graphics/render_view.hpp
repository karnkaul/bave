#pragma once
#include <bave/graphics/rect.hpp>
#include <bave/graphics/transform.hpp>

namespace bave {
/// \brief Orthographic Z plane bounds.
struct ZPlane {
	static constexpr auto default_v{1000.0f};

	float near{-default_v};
	float far{default_v};
};

/// \brief Rendering view.
struct RenderView {
	/// \brief View transform.
	Transform transform{};
	/// \brief Size of world in view.
	glm::vec2 viewport{};
	/// \brief Z Plane of view.
	ZPlane z_plane{};
	/// \brief Scissor of view in normalized coords [0-1].
	Rect<> n_scissor{uv_rect_v};

	/// \brief Unproject a point in normalized coords [0-1] to view space.
	/// \param ndc Point in normalized device coordinates.
	/// \returns Unprojected point in view space.
	[[nodiscard]] auto unproject(glm::vec2 ndc) const -> glm::vec2;
};
} // namespace bave
