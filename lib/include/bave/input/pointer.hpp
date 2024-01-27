#pragma once
#include <bave/input/action.hpp>
#include <glm/vec2.hpp>

namespace bave {
/// \brief Open set of pointer IDs.
///
/// On desktop there's always exactly one pointer: ePrimary.
/// On Android the first pointer in a gesture will be ePrimary,
/// subsequent ones monotonically incrementing.
enum struct PointerId : int { ePrimary = 0 };

/// \brief A pointer ID and its position.
struct Pointer {
	using Id = PointerId;

	/// \brief ID of pointer.
	Id id{};
	/// \brief Position in centered framebuffer coordinates.
	glm::vec2 position{};
};
} // namespace bave
