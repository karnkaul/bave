#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace bave {
template <typename Type = float>
struct Rect {
	glm::tvec2<Type> lt{};
	glm::tvec2<Type> rb{};

	static constexpr auto from_lbrt(glm::tvec2<Type> lb, glm::tvec2<Type> rt) -> Rect { return {.lt = {lb.x, rt.y}, .rb = {rt.x, lb.y}}; }

	static constexpr auto from_extent(glm::tvec2<Type> extent, glm::tvec2<Type> centre = {}) -> Rect {
		if (extent.x == Type{} && extent.y == Type{}) { return {.lt = centre, .rb = centre}; }
		auto const he = extent / Type{2};
		return {.lt = {centre.x - he.x, centre.y + he.y}, .rb = {centre.x + he.x, centre.y - he.y}};
	}

	[[nodiscard]] constexpr auto top_left() const -> glm::tvec2<Type> { return lt; }
	[[nodiscard]] constexpr auto top_right() const -> glm::tvec2<Type> { return {rb.x, lt.y}; }
	[[nodiscard]] constexpr auto bottom_left() const -> glm::tvec2<Type> { return {lt.x, rb.y}; }
	[[nodiscard]] constexpr auto bottom_right() const -> glm::tvec2<Type> { return rb; }

	[[nodiscard]] constexpr auto centre() const -> glm::tvec2<Type> { return {(lt.x + rb.x) / Type{2}, (lt.y + rb.y) / Type{2}}; }
	[[nodiscard]] constexpr auto extent() const -> glm::tvec2<Type> { return {rb.x - lt.x, lt.y - rb.y}; }

	[[nodiscard]] constexpr auto contains(glm::tvec2<Type> const point) const -> bool {
		return lt.x <= point.x && point.x <= rb.x && rb.y <= point.y && point.y <= lt.y;
	}

	[[nodiscard]] constexpr auto contains(Rect<Type> const& other) const -> bool {
		return contains(other.top_left()) || contains(other.top_right()) || contains(other.bottom_left()) || contains(other.bottom_right());
	}

	template <typename T>
	constexpr operator Rect<T>() const {
		return {lt, rb};
	}

	constexpr auto operator*=(float const scale) -> Rect& {
		lt *= scale;
		rb *= scale;
		return *this;
	}

	friend constexpr auto operator*(float const scale, Rect const& rect) {
		auto ret = rect;
		ret *= scale;
		return ret;
	}

	auto operator==(Rect const&) const -> bool = default;
};

template <typename Type>
[[nodiscard]] constexpr auto is_intersecting(Rect<Type> const& a, Rect<Type> const& b) -> bool {
	return a.contains(b) || b.contains(a);
}

using OffsetRect = Rect<std::int32_t>;

using UvRect = Rect<float>;

inline constexpr UvRect uv_rect_v{.lt = {0.0f, 0.0f}, .rb = {1.0f, 1.0f}};
} // namespace bave
