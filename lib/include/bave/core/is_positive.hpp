#pragma once
#include <glm/vec2.hpp>

namespace bave {
template <typename Type>
[[nodiscard]] constexpr auto is_positive(glm::tvec2<Type> const extent) -> bool {
	return extent.x > Type{0} && extent.y > Type{0};
}
} // namespace bave
