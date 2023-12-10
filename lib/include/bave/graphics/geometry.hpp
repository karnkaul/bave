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
struct Quad {
	glm::vec2 size{1.0f};
	UvRect uv{uv_rect_v};
	Rgba rgba{white_v};
	glm::vec3 origin{};

	auto operator==(Quad const&) const -> bool = default;
};

struct Circle {
	static constexpr int resolution_v{128};

	float diameter{1.0f};
	int resolution{resolution_v};
	Rgba rgba{white_v};
	glm::vec3 origin{};

	auto operator==(Circle const&) const -> bool = default;
};

struct RoundedQuad : Quad {
	float corner_radius{0.25f};
	int corner_resolution{8};

	auto operator==(RoundedQuad const&) const -> bool = default;
};

struct NineSlice {
	struct Size {
		glm::vec2 total{1.0f};
		glm::vec2 top{0.25f};
		glm::vec2 bottom{0.25f};

		[[nodiscard]] auto rescaled(glm::vec2 extent) const -> Size;

		auto operator==(Size const&) const -> bool = default;
	};

	Size size{};
	UvRect top_uv{.lt = {}, .rb = glm::vec2{0.25f}};
	UvRect bottom_uv{.lt = glm::vec2{0.75f}, .rb = glm::vec2{1.0f}};
	Rgba rgba{white_v};
	glm::vec3 origin{};

	auto operator==(NineSlice const&) const -> bool = default;
};

struct Vertex {
	glm::vec2 position{};
	glm::vec2 uv{};
	glm::vec4 rgba{1.0f};
};

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
} // namespace bave
