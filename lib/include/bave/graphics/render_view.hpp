#pragma once
#include <bave/transform.hpp>
#include <optional>

namespace bave {
struct ZPlane {
	static constexpr auto default_v{1000.0f};

	float near{-default_v};
	float far{default_v};
};

struct RenderView {
	Transform transform{};
	std::optional<glm::vec2> viewport{};
	ZPlane z_plane{};
};
} // namespace bave
