#pragma once
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/topology.hpp>
#include <bave/graphics/transform.hpp>
#include <span>
#include <vector>

namespace bave {
/// \brief A single render instance.
struct RenderInstance {
	struct Baked;
	struct List;

	/// \brief World transform of this instance.
	Transform transform{};
	/// \brief Tint to apply during draw.
	Rgba tint{};

	[[nodiscard]] auto to_baked(glm::mat4 const& parent = glm::identity<glm::mat4>()) const -> Baked;

	static void fill_baked(std::vector<Baked>& out, std::span<RenderInstance const> instances, glm::mat4 const& parent);
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

inline auto RenderInstance::to_baked(glm::mat4 const& parent) const -> Baked {
	return Baked{.transform = parent * transform.matrix(), .rgba = Rgba::to_linear(tint.to_vec4())};
}

inline void RenderInstance::fill_baked(std::vector<Baked>& out, std::span<RenderInstance const> instances, glm::mat4 const& parent) {
	out.reserve(out.size() + instances.size());
	for (auto const& instance : instances) { out.push_back(instance.to_baked(parent)); }
}
} // namespace bave
