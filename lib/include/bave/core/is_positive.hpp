#pragma once
#include <glm/vec2.hpp>

namespace bave {
[[nodiscard]] constexpr auto is_positive(glm::vec2 const extent) -> bool { return extent.x > 0.0f && extent.y > 0.0f; }
} // namespace bave
