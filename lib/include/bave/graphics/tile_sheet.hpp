#pragma once
#include <bave/core/ptr.hpp>
#include <bave/graphics/rect.hpp>
#include <string>
#include <vector>

namespace bave {
struct TileBlock {
	std::string id{};
	Rect<int> image_rect{};
	std::vector<Rect<int>> colliders{};
};

struct TileSheet {
	struct Tile {
		std::string id{};
		Rect<int> image_rect{};
		std::vector<Rect<int>> colliders{};

		[[nodiscard]] auto get_size() const -> glm::ivec2;
		[[nodiscard]] auto get_uv(glm::vec2 image_size) const -> UvRect;
	};

	std::vector<Tile> tiles{};

	[[nodiscard]] static auto make_rects(glm::ivec2 size, glm::ivec2 tile_count) -> std::vector<Rect<int>>;

	[[nodiscard]] auto find_tile(std::string_view id) const -> Ptr<Tile const>;
};
} // namespace bave
