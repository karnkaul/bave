#pragma once
#include <bave/graphics/texture.hpp>
#include <bave/graphics/tile_sheet.hpp>

namespace bave {
/// \brief Texture atlas (sprite sheet).
class TextureAtlas : public Texture {
  public:
	/// \brief Constructor.
	/// \param render_device Non-null pointer to RenderDevice.
	/// \param bitmap View of atlas bitmap.
	/// \param sheet TileSheet describing tiles and their rects.
	/// \param mip_map Whether to enable mip-mapping.
	explicit TextureAtlas(NotNull<RenderDevice*> render_device, BitmapView bitmap, TileSheet sheet, bool mip_map = false);

	[[nodiscard]] auto get_sheet() const -> TileSheet const& { return m_sheet; }
	[[nodiscard]] auto get_uv(std::string_view id) const -> UvRect;

  private:
	TileSheet m_sheet;
};
} // namespace bave
