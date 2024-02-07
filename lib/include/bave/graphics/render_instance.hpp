#pragma once
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/topology.hpp>
#include <bave/graphics/transform.hpp>
#include <span>

namespace bave {
/// \brief A single render instance.
struct RenderInstance {
	struct Baked;

	Transform transform{};
	Rgba tint{};

	[[nodiscard]] auto bake() const -> Baked;
};

/// \brief Baked render instance (ready to upload to GPU).
struct RenderInstance::Baked {
	glm::mat4 transform;
	glm::vec4 rgba;
};

/// \brief View of a draw primitive.
struct RenderPrimitive {
	std::span<std::byte const> bytes{};
	std::size_t ibo_offset{};
	std::uint32_t vertices{};
	std::uint32_t indices{};
	Topology topology{Topology::eTriangleList};
};

inline auto RenderInstance::bake() const -> Baked { return Baked{.transform = transform.matrix(), .rgba = Rgba::to_linear(tint.to_vec4())}; }
} // namespace bave
