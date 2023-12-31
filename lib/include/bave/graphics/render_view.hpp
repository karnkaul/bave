#pragma once
#include <bave/graphics/transform.hpp>

namespace bave {
struct ZPlane {
	static constexpr auto default_v{1000.0f};

	float near{-default_v};
	float far{default_v};
};

struct RenderView {
	Transform transform{};
	glm::vec2 viewport{};
	ZPlane z_plane{};

	[[nodiscard]] auto unproject(glm::vec2 ndc) const -> glm::vec2;
};
} // namespace bave
