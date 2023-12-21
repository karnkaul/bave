#include <bave/graphics/projector.hpp>
#include <bave/graphics/render_view.hpp>

namespace bave {
auto RenderView::unproject(glm::vec2 const ndc) const -> glm::vec2 {
	auto point = 0.5f * viewport * ndc;
	return point;
}
} // namespace bave
