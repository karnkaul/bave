#pragma once
#include <bave/graphics/shape.hpp>
#include <bave/graphics/texture_9slice.hpp>

namespace bave {
/// \brief 9-sliced sprite.
class Sprite9Slice : public NineQuadShape {
  public:
	/// \brief Set 9-sliced texture. Replaces primary texture.
	/// \param texture Texture9Slice to set.
	void set_texture_9slice(std::shared_ptr<Texture9Slice const> texture);
	/// \brief Resize keeping 9-slice intact.
	/// \param size Size to set.
	void set_size(glm::vec2 size);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_shape().size.current; }
};
} // namespace bave
