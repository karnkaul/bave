#include <bave/graphics/geometry.hpp>
#include <algorithm>
#include <array>

namespace bave {
namespace {
struct Arc {
	Degrees start{};
	Degrees finish{};
};

struct Sector {
	glm::vec2 origin{};
	glm::vec4 rgba{};
	float radius{};
	Arc arc{};
	Degrees step{};
	UvRect uv{uv_rect_v};
};

struct Sliced {
	NineSlice const& nine_slice; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
	int corner_resolution{};
};

auto append_sector(Geometry& out, Sector const& sector) -> void {
	auto const& o = sector.origin;
	auto const uvo = sector.uv.centre();
	auto const uvhe = 0.5f * sector.uv.extent();
	auto const add_tri = [&](glm::vec2 const prev, glm::vec2 const curr) {
		// NOLINTNEXTLINE
		Vertex const vs[] = {
			{o, uvo, sector.rgba},
			{o + glm::vec2{sector.radius * prev}, uvo + prev * uvhe, sector.rgba},
			{o + glm::vec2{sector.radius * curr}, uvo + curr * uvhe, sector.rgba},
		};
		// NOLINTNEXTLINE
		std::uint32_t const is[] = {0, 1, 2};
		out.append(vs, is);
	};
	auto deg = sector.arc.start;
	auto rad = deg.to_radians();
	auto const get_dir = [](Radians const r) { return glm::vec2{-std::sin(r), std::cos(r)}; };
	auto prev = get_dir(rad);
	// NOLINTNEXTLINE
	for (deg = sector.arc.start + sector.step; deg < sector.arc.finish; deg.value += sector.step) {
		rad = deg.to_radians();
		auto const curr = get_dir(rad);
		add_tri(prev, curr);
		prev = curr;
	}
	add_tri(prev, get_dir(sector.arc.finish));
}

auto append_sliced(Geometry& out, Sliced const& sliced) -> void {
	/*
		cells and their addresses:
		+----+--------+----+
		| 00 |   01   | 02 |
		+----+--------+----+
		|    |        |    |
		| 10 |   11   | 12 |
		|    |        |    |
		+----+--------+----+
		| 20 |   21   | 22 |
		+----+--------+----+

		corner cells: 00, 02, 20, 22 (these retain their size and UVs on size.total changing)
	*/

	auto const& nine_slice = sliced.nine_slice;
	auto const half_size = 0.5f * nine_slice.size.total;
	auto const corner_uvs = std::array{
		nine_slice.top_uv,																											  // 00
		UvRect{.lt = {nine_slice.bottom_uv.lt.x, nine_slice.top_uv.lt.y}, .rb = {nine_slice.bottom_uv.rb.x, nine_slice.top_uv.rb.y}}, // 02
		UvRect{.lt = {nine_slice.top_uv.lt.x, nine_slice.bottom_uv.lt.y}, .rb = {nine_slice.top_uv.rb.x, nine_slice.bottom_uv.rb.y}}, // 20
		nine_slice.bottom_uv,																										  // 22
	};
	// origin offsets
	auto const cell_offsets = std::array{
		glm::vec2{-half_size.x + 0.5f * nine_slice.size.left_top.x, half_size.y - 0.5f * nine_slice.size.left_top.y}, // 00
		glm::vec2{0.5f * (nine_slice.size.left_top.x - nine_slice.size.right_bottom.x),
				  0.5f * (nine_slice.size.right_bottom.y - nine_slice.size.left_top.y)},													// 11
		glm::vec2{half_size.x - 0.5f * nine_slice.size.right_bottom.x, 0.5f * (-nine_slice.size.total.y + nine_slice.size.right_bottom.y)}, // 22
	};
	// rounded corner arcs
	auto const corner_arcs = std::array{
		Arc{.start = Degrees{0.0f}, .finish = Degrees{90.0f}},	  // 00
		Arc{.start = Degrees{270.0f}, .finish = Degrees{360.0f}}, // 02
		Arc{.start = Degrees{90.0f}, .finish = Degrees{180.0f}},  // 20
		Arc{.start = Degrees{180.0f}, .finish = Degrees{270.0f}}, // 22
	};
	// rounded corner offsets
	auto const corner_offsets = std::array{
		0.5f * glm::vec2{nine_slice.size.left_top.x, -nine_slice.size.left_top.y},		   // 00
		0.5f * glm::vec2{-nine_slice.size.right_bottom.x, -nine_slice.size.left_top.y},	   // 02
		0.5f * glm::vec2{nine_slice.size.left_top.x, nine_slice.size.right_bottom.y},	   // 20
		0.5f * glm::vec2{-nine_slice.size.right_bottom.x, nine_slice.size.right_bottom.y}, // 22
	};

	auto const rgba = Rgba::to_linear(nine_slice.rgba.to_vec4());
	auto const append_corner = [&](std::size_t index, glm::vec2 size, UvRect uv, glm::vec2 origin) {
		if (sliced.corner_resolution > 1) {
			auto const arc = corner_arcs.at(index);
			auto const step = Degrees{(arc.finish - arc.start) / static_cast<float>(sliced.corner_resolution)};
			uv.lt *= 2.0f;
			uv.rb *= 2.0f;
			auto const sector = Sector{
				.origin = origin + corner_offsets.at(index),
				.rgba = rgba,
				.radius = std::min(size.x, size.y),
				.arc = arc,
				.step = step,
				.uv = uv,
			};
			append_sector(out, sector);
		} else {
			out.append(Quad{.size = size, .uv = uv, .rgba = nine_slice.rgba, .origin = origin});
		}
	};

	auto const size_00 = nine_slice.size.left_top;
	auto const origin_00 = nine_slice.origin + glm::vec2{cell_offsets[0].x, cell_offsets[0].y};
	append_corner(0, size_00, corner_uvs[0], origin_00);

	auto const size_01 = glm::vec2{nine_slice.size.total.x - (nine_slice.size.left_top.x + nine_slice.size.right_bottom.x), nine_slice.size.left_top.y};
	auto const origin_01 = nine_slice.origin + glm::vec2{cell_offsets[1].x, cell_offsets[0].y};
	auto const uv_01 = UvRect{.lt = corner_uvs[0].top_right(), .rb = corner_uvs[1].bottom_left()};
	out.append(Quad{.size = size_01, .uv = uv_01, .rgba = nine_slice.rgba, .origin = origin_01});

	auto const size_02 = glm::vec2{nine_slice.size.right_bottom.x, nine_slice.size.left_top.y};
	auto const origin_02 = nine_slice.origin + glm::vec2{cell_offsets[2].x, cell_offsets[0].y};
	append_corner(1, size_02, corner_uvs[1], origin_02);

	auto const size_10 = glm::vec2{nine_slice.size.left_top.x, nine_slice.size.total.y - (nine_slice.size.left_top.y + nine_slice.size.right_bottom.y)};
	auto const origin_10 = nine_slice.origin + glm::vec2{cell_offsets[0].x, cell_offsets[1].y};
	auto const uv_10 = UvRect{.lt = corner_uvs[0].bottom_left(), .rb = corner_uvs[2].top_right()};
	out.append(Quad{.size = size_10, .uv = uv_10, .rgba = nine_slice.rgba, .origin = origin_10});

	auto const size_11 = nine_slice.size.total - (nine_slice.size.left_top + nine_slice.size.right_bottom);
	auto const origin_11 = nine_slice.origin + glm::vec2{cell_offsets[1].x, cell_offsets[1].y};
	auto const uv_11 = UvRect{.lt = corner_uvs[0].bottom_right(), .rb = corner_uvs[3].top_left()};
	out.append(Quad{.size = size_11, .uv = uv_11, .rgba = nine_slice.rgba, .origin = origin_11});

	auto const size_12 = glm::vec2{nine_slice.size.right_bottom.x, nine_slice.size.total.y - (nine_slice.size.left_top.y + nine_slice.size.right_bottom.y)};
	auto const origin_12 = nine_slice.origin + glm::vec2{cell_offsets[2].x, cell_offsets[1].y};
	auto const uv_12 = UvRect{.lt = corner_uvs[1].bottom_left(), .rb = corner_uvs[3].top_right()};
	out.append(Quad{.size = size_12, .uv = uv_12, .rgba = nine_slice.rgba, .origin = origin_12});

	auto const size_20 = glm::vec2{nine_slice.size.left_top.x, nine_slice.size.right_bottom.y};
	auto const origin_20 = nine_slice.origin + glm::vec2{cell_offsets[0].x, cell_offsets[2].y};
	append_corner(2, size_20, corner_uvs[2], origin_20);

	auto const size_21 = glm::vec2{nine_slice.size.total.x - (nine_slice.size.left_top.x + nine_slice.size.right_bottom.x), nine_slice.size.right_bottom.y};
	auto const origin_21 = nine_slice.origin + glm::vec2{cell_offsets[1].x, cell_offsets[2].y};
	auto const uv_21 = UvRect{.lt = corner_uvs[2].top_right(), .rb = corner_uvs[3].bottom_left()};
	out.append(Quad{.size = size_21, .uv = uv_21, .rgba = nine_slice.rgba, .origin = origin_21});

	auto const size_22 = nine_slice.size.right_bottom;
	auto const origin_22 = nine_slice.origin + glm::vec2{cell_offsets[2].x, cell_offsets[2].y};
	append_corner(3, size_22, corner_uvs[3], origin_22);
}
} // namespace

auto Geometry::append(std::span<Vertex const> vs, std::span<std::uint32_t const> is) -> Geometry& {
	auto const i_offset = static_cast<std::uint32_t>(vertices.size());
	vertices.reserve(vertices.size() + vs.size());
	std::copy(vs.begin(), vs.end(), std::back_inserter(vertices));
	std::transform(is.begin(), is.end(), std::back_inserter(indices), [i_offset](std::uint32_t const i) { return i + i_offset; });
	return *this;
}

auto Geometry::append(Quad const& quad) -> Geometry& {
	if (quad.size.x <= 0.0f || quad.size.y <= 0.0f) { return *this; }
	auto const h = 0.5f * quad.size;
	auto const& o = quad.origin;
	auto const rgba = Rgba::to_linear(quad.rgba.to_vec4());
	// NOLINTNEXTLINE
	Vertex const vs[] = {
		{{o.x - h.x, o.y + h.y}, quad.uv.top_left(), rgba},
		{{o.x + h.x, o.y + h.y}, quad.uv.top_right(), rgba},
		{{o.x + h.x, o.y - h.y}, quad.uv.bottom_right(), rgba},
		{{o.x - h.x, o.y - h.y}, quad.uv.bottom_left(), rgba},
	};
	// NOLINTNEXTLINE
	std::uint32_t const is[] = {
		0, 1, 2, 2, 3, 0,
	};
	return append(vs, is);
}

auto Geometry::append(Circle const& circle) -> Geometry& {
	if (circle.diameter <= 0.0f) { return *this; }
	auto const finish = Degrees{360.0f};
	auto const resolution = circle.resolution;
	auto const sector = Sector{
		.origin = circle.origin,
		.rgba = Rgba::to_linear(circle.rgba.to_vec4()),
		.radius = 0.5f * circle.diameter,
		.arc = {.finish = finish},
		.step = Degrees{finish / static_cast<float>(resolution)},
	};
	append_sector(*this, sector);
	return *this;
}

auto Geometry::append(RoundedQuad const& rounded_quad) -> Geometry& {
	if (rounded_quad.size.x <= 0.0f || rounded_quad.size.y <= 0.0f) { return *this; }
	if (rounded_quad.corner_radius <= 0.0f) { return append(static_cast<Quad const&>(rounded_quad)); }
	auto const corner_size = glm::vec2{rounded_quad.corner_radius};
	auto const n_corner_size = corner_size / rounded_quad.size;
	auto const nine_slice = NineSlice{
		.size = NineSlice::Size{.total = rounded_quad.size, .left_top = corner_size, .right_bottom = corner_size},
		.top_uv = UvRect{.lt = rounded_quad.uv.lt, .rb = rounded_quad.uv.lt + n_corner_size},
		.bottom_uv = UvRect{.lt = rounded_quad.uv.rb - n_corner_size, .rb = rounded_quad.uv.rb},
		.rgba = rounded_quad.rgba,
		.origin = rounded_quad.origin,
	};
	auto sliced = Sliced{.nine_slice = nine_slice, .corner_resolution = rounded_quad.corner_resolution};
	append_sliced(*this, sliced);
	return *this;
}

auto Geometry::append(NineSlice const& nine_slice) -> Geometry& {
	if (nine_slice.size.total.x <= 0.0f || nine_slice.size.total.y <= 0.0f) { return *this; }
	auto const sliced = Sliced{.nine_slice = nine_slice};
	append_sliced(*this, sliced);
	return *this;
}
} // namespace bave
