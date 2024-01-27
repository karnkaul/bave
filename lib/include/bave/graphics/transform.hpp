#pragma once
#include <bave/core/radians.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace bave {
/// \brief 2D transformation data.
///
/// All transformations are right-handed: +x is towards the right, +y is upwards.
/// Positive rotation is counter-clockwise. (As +z points out of the screen.)
/// Position is centred in world-space.
struct Transform {
	glm::vec2 position{};
	Radians rotation{};
	glm::vec2 scale{1.0f};

	/// \brief Build 4D transformation matrix.
	[[nodiscard]] auto matrix() const -> glm::mat4;
};

/// \brief Build 4D rotation matrix.
/// \param rotation Rotation around z-axis.
auto get_rotation_matrix(Radians rotation) -> glm::mat4;

static_assert(std::is_trivially_copyable_v<Transform>);
} // namespace bave
