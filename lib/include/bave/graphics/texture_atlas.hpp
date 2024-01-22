#pragma once
#include <bave/graphics/texture.hpp>
#include <bave/graphics/tile_sheet.hpp>

namespace bave {
class TextureAtlas : public Texture {
  public:
	struct Block {
		std::string id{};
		Rect<int> image_rect{};
	};

	struct Tile {
		glm::ivec2 size{};
		UvRect uv{};
		glm::ivec2 top_left{};
	};

	explicit TextureAtlas(NotNull<RenderDevice*> render_device, BitmapView bitmap, TileSheet sheet, bool mip_map = false);

	[[nodiscard]] auto get_sheet() const -> TileSheet const& { return m_sheet; }
	[[nodiscard]] auto get_uv(std::string_view id) const -> UvRect;

  private:
	TileSheet m_sheet;
};
} // namespace bave
