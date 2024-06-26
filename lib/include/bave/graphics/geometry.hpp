#pragma once
#include <bave/core/radians.hpp>
#include <bave/graphics/rect.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/topology.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <span>
#include <vector>

namespace bave {
/// \brief A single 2D vertex.
struct Vertex {
	glm::vec2 position{};
	glm::vec2 uv{};
	glm::vec4 rgba{1.0f};
};

struct Quad;
struct Circle;
struct RoundedQuad;
struct NineQuad;
struct LineRect;

/// \brief Collection of vertices and indices representing a graphics draw primitive.
struct VertexArray {
	std::vector<Vertex> vertices{};
	std::vector<std::uint32_t> indices{};

	auto append(std::span<Vertex const> vs, std::span<std::uint32_t const> is) -> VertexArray&;

	auto append(Quad const& quad) -> VertexArray&;
	auto append(Circle const& circle) -> VertexArray&;
	auto append(RoundedQuad const& rounded_quad) -> VertexArray&;
	auto append(NineQuad const& nine_quad) -> VertexArray&;
	auto append(LineRect const& rect) -> VertexArray&;

	[[nodiscard]] auto is_empty() const -> bool { return vertices.empty(); }
	[[nodiscard]] auto has_indices() const -> bool { return !indices.empty(); }
};

/// \brief VertexArray and its Topology.
struct Geometry {
	VertexArray vertex_array{};
	Topology topology{Topology::eTriangleList};

	template <typename ShapeT>
	static auto from(ShapeT const& shape, Topology const toplogy = Topology::eTriangleList) -> Geometry {
		auto ret = Geometry{};
		ret.vertex_array.append(shape);
		ret.topology = toplogy;
		return ret;
	}
};

/// \brief Spec for an axis-aligned quad.
struct Quad {
	static constexpr auto size_v = glm::vec2{200.0f};

	glm::vec2 size{size_v};
	UvRect uv{uv_rect_v};
	Rgba rgba{white_v};
	glm::vec2 origin{};

	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(Quad const&) const -> bool = default;
};

/// \brief Spec for a circle.
struct Circle {
	static constexpr int resolution_v{128};
	static constexpr auto diameter_v{200.0f};

	float diameter{diameter_v};
	int resolution{resolution_v};
	Rgba rgba{white_v};
	glm::vec2 origin{};

	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(Circle const&) const -> bool = default;
};

/// \brief Spec for a quad with rounded corners.
struct RoundedQuad : Quad {
	float corner_radius{0.25f * size_v.x};
	int corner_resolution{8};

	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(RoundedQuad const&) const -> bool = default;
};

/// \brief Spec for a 9-slice.
struct NineSlice {
	glm::vec2 n_left_top{0.25f};
	glm::vec2 n_right_bottom{0.75f};

	auto operator==(NineSlice const&) const -> bool = default;
};

/// \brief Spec for a 9-sliced axis-aligned quad.
struct NineQuad {
	struct Size {
		glm::vec2 reference{Quad::size_v};
		glm::vec2 current{Quad::size_v};

		constexpr Size(glm::vec2 const size = Quad::size_v) : Size(size, size) {}
		constexpr Size(glm::vec2 const reference, glm::vec2 const current) : reference(reference), current(current) {}

		auto operator==(Size const&) const -> bool = default;
	};

	Size size{};
	NineSlice slice{};
	Rgba rgba{white_v};
	glm::vec2 origin{};

	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(NineQuad const&) const -> bool = default;
};

/// \brief Spec for a quad as a line strip.
struct LineRect : Quad {
	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this, Topology::eLineStrip); }
};
} // namespace bave
