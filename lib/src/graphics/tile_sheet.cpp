#include <bave/core/is_positive.hpp>
#include <bave/graphics/tile_sheet.hpp>
#include <algorithm>

namespace bave {
auto TileSheet::make_rects(glm::ivec2 size, glm::ivec2 tile_count) -> std::vector<Rect<int>> {
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

auto TileSheet::Tile::get_size() const -> glm::ivec2 { return {image_rect.rb.x - image_rect.lt.x, image_rect.rb.y - image_rect.lt.y}; }

auto TileSheet::Tile::get_uv(glm::vec2 const image_size) const -> UvRect {
	if (!is_positive(image_size)) { return uv_rect_v; }
	return {.lt = glm::vec2{image_rect.lt} / image_size, .rb = glm::vec2{image_rect.rb} / image_size};
}

auto TileSheet::find_tile(std::string_view const id) const -> Ptr<Tile const> {
	auto const it = std::find_if(tiles.begin(), tiles.end(), [id](Tile const& t) { return t.id == id; });
	if (it == tiles.end()) { return {}; }
	return &*it;
}
} // namespace bave
