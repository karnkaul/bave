#include <bave/graphics/texture_atlas.hpp>

namespace bave {
TextureAtlas::TextureAtlas(NotNull<RenderDevice*> render_device, BitmapView bitmap, TileSheet sheet, bool mip_map)
	: Texture(render_device, bitmap, mip_map), m_sheet(std::move(sheet)) {}

auto TextureAtlas::get_uv(std::string_view const id) const -> UvRect {
	auto const* tile = m_sheet.find_tile(id);
	if (tile == nullptr) { return uv_rect_v; }
	return tile->get_uv(get_size());
}
} // namespace bave
