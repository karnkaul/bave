#pragma once
#include <bave/core/radians.hpp>
#include <bave/graphics/rect.hpp>
#include <bave/graphics/rgba.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <span>
#include <vector>

namespace bave {
struct Vertex {
	glm::vec2 position{};
	glm::vec2 uv{};
	glm::vec4 rgba{1.0f};
};

struct Quad;
struct Circle;
struct RoundedQuad;
struct NineSlice;

struct Geometry {
	std::vector<Vertex> vertices{};
	std::vector<std::uint32_t> indices{};

	auto append(std::span<Vertex const> vs, std::span<std::uint32_t const> is) -> Geometry&;

	auto append(Quad const& quad) -> Geometry&;
	auto append(Circle const& circle) -> Geometry&;
	auto append(RoundedQuad const& rounded_quad) -> Geometry&;
	auto append(NineSlice const& nine_slice) -> Geometry&;

	template <typename ShapeT>
	static auto from(ShapeT const& shape) -> Geometry {
		auto ret = Geometry{};
		ret.append(shape);
		return ret;
	}
};

struct Quad {
	static constexpr auto size_v = glm::vec2{200.0f};

	glm::vec2 size{size_v};
	UvRect uv{uv_rect_v};
	Rgba rgba{white_v};
	glm::vec2 origin{};

	[[nodiscard]] constexpr auto get_bounds(glm::vec2 const position) const -> Rect<> { return Rect<>::from_extent(size, origin + position); }
	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(Quad const&) const -> bool = default;
};

struct Circle {
	static constexpr int resolution_v{128};
	static constexpr auto diameter_v{200.0f};

	float diameter{diameter_v};
	int resolution{resolution_v};
	Rgba rgba{white_v};
	glm::vec2 origin{};

	[[nodiscard]] constexpr auto get_bounds(glm::vec2 const position) const -> Rect<> { return Rect<>::from_extent(glm::vec2{diameter}, origin + position); }

	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(Circle const&) const -> bool = default;
};

struct RoundedQuad : Quad {
	float corner_radius{0.25f * size_v.x};
	int corner_resolution{8};

	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(RoundedQuad const&) const -> bool = default;
};

struct NineSlice {
	struct Size {
		glm::vec2 total{Quad::size_v};
		glm::vec2 left_top{0.25f * Quad::size_v};
		glm::vec2 right_bottom{0.25f * Quad::size_v};

		[[nodiscard]] auto rescaled(glm::vec2 extent) const -> Size;

		auto operator==(Size const&) const -> bool = default;
	};

	Size size{};
	UvRect top_uv{.lt = {}, .rb = glm::vec2{0.25f}};
	UvRect bottom_uv{.lt = glm::vec2{0.75f}, .rb = glm::vec2{1.0f}};
	Rgba rgba{white_v};
	glm::vec2 origin{};

	[[nodiscard]] constexpr auto get_bounds(glm::vec2 const position) const -> Rect<> { return Rect<>::from_extent(size.total, origin + position); }
	[[nodiscard]] auto to_geometry() const -> Geometry { return Geometry::from(*this); }

	auto operator==(NineSlice const&) const -> bool = default;
};
} // namespace bave
