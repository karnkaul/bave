#pragma once
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/transform.hpp>
#include <span>

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

struct RenderPrimitive {
	std::span<std::byte const> bytes{};
	std::size_t ibo_offset{};
	std::uint32_t vertices{};
	std::uint32_t indices{};
};

inline auto RenderInstance::bake() const -> Baked { return Baked{.transform = transform.matrix(), .rgba = Rgba::to_linear(tint.to_vec4())}; }
} // namespace bave
