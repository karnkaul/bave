#include <bave/graphics/projector.hpp>
#include <bave/graphics/render_view.hpp>

namespace bave {
auto RenderView::unproject(glm::vec2 const ndc) const -> glm::vec2 {
	static constexpr auto m = glm::identity<glm::mat4>();
	auto point = viewport * ndc;
	auto const scale = glm::scale(m, 1.0f / glm::vec3{transform.scale, 1.0f});
	auto const rotation = get_rotation_matrix(transform.rotation);
	auto const translation = glm::translate(m, {transform.position, 0.0f});
	point = scale * rotation * translation * glm::vec4{point, 0.0f, 1.0f};
	return point;
}
} // namespace bave
