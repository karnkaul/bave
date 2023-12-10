#pragma once
#include <bave/graphics/rgba.hpp>
#include <bave/transform.hpp>

namespace bave {
struct RenderInstance {
	struct Baked;

	Transform transform{};
	Rgba rgba{};

	[[nodiscard]] auto bake() const -> Baked;
};

struct RenderInstance::Baked {
	glm::mat4 transform;
	glm::vec4 rgba;
};

inline auto RenderInstance::bake() const -> Baked { return Baked{.transform = transform.matrix(), .rgba = Rgba::to_linear(rgba.to_vec4())}; }
} // namespace bave
