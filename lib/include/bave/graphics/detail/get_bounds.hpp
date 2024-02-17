#pragma once
#include <bave/graphics/rect.hpp>

namespace bave {
class Drawable;
struct Circle;
template <typename Type>
class Shape;

namespace detail {
[[nodiscard]] auto get_bounds(Drawable const& drawable) -> Rect<>;
[[nodiscard]] auto get_bounds(Shape<Circle> const& circle_shape) -> Rect<>;
} // namespace detail
} // namespace bave
