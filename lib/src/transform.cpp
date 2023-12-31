#include <bave/graphics/transform.hpp>
#include <glm/gtx/transform.hpp>

namespace bave {
auto Transform::matrix() const -> glm::mat4 {
	static constexpr auto m = glm::identity<glm::mat4>();
	return glm::translate(m, glm::vec3{position, 0.0f}) * get_rotation_matrix(rotation) * glm::scale(m, glm::vec3{scale, 1.0f});
}
} // namespace bave

auto bave::get_rotation_matrix(Radians const rotation) -> glm::mat4 {
	auto const s = glm::sin(rotation);
	auto const c = glm::cos(rotation);
	return glm::mat4{
		glm::vec4{c, s, 0.0f, 0.0f},
		glm::vec4{-s, c, 0.0f, 0.0f},
		glm::vec4{0.0f, 0.0f, 1.0f, 0.0f},
		glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
	};
}
