#pragma once
#include <bave/graphics/shape.hpp>
#include <bave/graphics/texture_atlas.hpp>
#include <bave/graphics/tile_sheet.hpp>
#include <optional>

namespace bave {
/// \brief Drawable sprite.
class Sprite : public QuadShape {
  public:
	/// \brief Set the exact size of the sprite.
	void set_size(glm::vec2 size);
	/// \brief Set the max size of the sprite.
	///
	/// The current size may change to match a Tile's aspect ratio,
	/// if so it will fit within max_size.
	void set_auto_size(glm::vec2 max_size);

	/// \brief Set the UV rect.
	void set_uv(UvRect uv);
	/// \brief Reset the UV rect.
	void reset_uv() { set_uv(uv_rect_v); }

	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_shape().size; }
	[[nodiscard]] auto get_uv() const -> UvRect const& { return get_shape().uv; }

	/// \brief Match Tile.
	///
	/// Sets UV, and resizes if auto_size has been set.
	void set_tile(TileSheet::Tile const& tile);

  protected:
	void set_tile(TileSheet::Tile const& tile, glm::ivec2 total_size);

	std::optional<glm::vec2> m_max_size{};
};
} // namespace bave
