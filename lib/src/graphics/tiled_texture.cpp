#include <bave/core/is_positive.hpp>
#include <bave/data_store.hpp>
#include <bave/graphics/image_file.hpp>
#include <bave/graphics/tiled_texture.hpp>
#include <bave/json_io.hpp>

namespace bave {
namespace {
constexpr auto make_tile(Rect<int> const& rect, glm::vec2 const size) {
	auto ret = TiledTexture::Tile{.size = {rect.rb.x - rect.lt.x, rect.rb.y - rect.lt.y}};
	ret.uv = Rect<>{.lt = glm::vec2{rect.lt} / size, .rb = glm::vec2{rect.rb} / size};
	return ret;
}
} // namespace

auto TiledTexture::make_rects(glm::ivec2 size, glm::ivec2 tile_count) -> std::vector<Rect<int>> {
	if (!is_positive(size) || !is_positive(tile_count)) { return {}; }

	auto const tile_size = size / tile_count;
	auto ret = std::vector<Rect<int>>{};
	ret.reserve(static_cast<std::size_t>(tile_count.x * tile_count.y));
	auto tile = Rect<int>{.rb = tile_size};
	for (int row = 0; row < tile_count.y; ++row) {
		for (int col = 0; col < tile_count.x; ++col) {
			ret.push_back(tile);
			tile.lt.x += tile_size.x;
			tile.rb.x += tile_size.x;
		}
		tile.lt.x = 0;
		tile.rb.x = tile_size.x;
		tile.lt.y += tile_size.y;
		tile.rb.y += tile_size.y;
	}
	return ret;
}

TiledTexture::TiledTexture(NotNull<RenderDevice*> render_device, BitmapView bitmap, std::vector<Block> blocks, bool mip_map)
	: Texture(render_device, bitmap, mip_map), m_blocks(std::move(blocks)) {
	for (auto const& block : m_blocks) {
		if (block.id.empty()) { continue; }
		m_rects.insert_or_assign(block.id, block.rect);
	}
}

auto TiledTexture::find_tile(std::string_view const id) const -> std::optional<Tile> {
	if (id.empty()) { return {}; }

	auto const it = m_rects.find(id);
	if (it == m_rects.end()) { return {}; }

	return make_tile(it->second, get_size());
}
} // namespace bave
