#include <bave/graphics/detail/get_bounds.hpp>
#include <bave/graphics/shape.hpp>

namespace bave {
auto detail::get_bounds(Drawable const& drawable) -> Rect<> {
	auto const matrix = drawable.transform.matrix();
	auto ret = Rect<>{};
	ret.lt.x = ret.rb.y = std::numeric_limits<float>::max();
	ret.lt.y = ret.rb.x = -std::numeric_limits<float>::max();
	for (auto const& vertex : drawable.get_geometry().vertex_array.vertices) {
		auto const position = glm::vec2{matrix * glm::vec4{vertex.position, 0.0f, 1.0f}};
		ret.lt.x = std::min(ret.lt.x, position.x);
		ret.lt.y = std::max(ret.lt.y, position.y);
		ret.rb.y = std::min(ret.rb.y, position.y);
		ret.rb.x = std::max(ret.rb.x, position.x);
	}
	return ret;
}

auto detail::get_bounds(Shape<Circle> const& circle_shape) -> Rect<> {
	auto const side = circle_shape.get_shape().diameter * circle_shape.transform.scale;
	return Rect<>::from_size(side, circle_shape.transform.position);
}
} // namespace bave
