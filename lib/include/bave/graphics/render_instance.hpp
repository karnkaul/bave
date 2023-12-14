#pragma once
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/transform.hpp>

namespace bave {
struct RenderInstance {
	struct Baked;

	Transform transform{};
	Rgba tint{};

	[[nodiscard]] auto bake() const -> Baked;
};

struct RenderInstance::Baked {
	glm::mat4 transform;
	glm::vec4 rgba;
};

inline auto RenderInstance::bake() const -> Baked { return Baked{.transform = transform.matrix(), .rgba = Rgba::to_linear(tint.to_vec4())}; }
} // namespace bave
