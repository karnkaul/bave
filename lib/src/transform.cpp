#include <bave/graphics/transform.hpp>
#include <glm/gtx/transform.hpp>

namespace bave {
auto Transform::matrix() const -> glm::mat4 {
	auto const s = glm::sin(rotation);
	auto const c = glm::cos(rotation);
	auto const rot = glm::mat4{
		glm::vec4{c, s, 0.0f, 0.0f},
		glm::vec4{-s, c, 0.0f, 0.0f},
		glm::vec4{0.0f, 0.0f, 1.0f, 0.0f},
		glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
	};
	return glm::translate(glm::mat4{1.0f}, glm::vec3{position, 0.0f}) * rot * glm::scale(glm::mat4{1.0f}, glm::vec3{scale, 1.0f});
}
} // namespace bave
