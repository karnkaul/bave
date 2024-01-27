#pragma once
#include <bave/core/ptr.hpp>
#include <bave/graphics/rect.hpp>
#include <string>
#include <vector>

namespace bave {
/// \brief A block describing a tile.
struct TileBlock {
	std::string id{};
	Rect<int> image_rect{};
	std::vector<Rect<int>> colliders{};
};

/// \brief A 2D sheet of tiles.
struct TileSheet {
	struct Tile {
		/// \brief Unique identifier of this tile.
		std::string id{};
		/// \brief Rect specifying sub-image.
		Rect<int> image_rect{};
		/// \brief Colliders in this tile.
		std::vector<Rect<int>> colliders{};

		[[nodiscard]] auto get_size() const -> glm::ivec2;
		/// \brief Get UV coordinates.
		/// \param image_size Total size of image.
		[[nodiscard]] auto get_uv(glm::vec2 image_size) const -> UvRect;
	};

	/// \brief Vector of tiles.
	std::vector<Tile> tiles{};

	/// \brief Create equi-spaced rects within a given space.
	/// \param size Space to create tiles in.
	/// \param tile_count Number of tiles to create.
	/// \returns tile_count equi-spaced rects covering size.
	[[nodiscard]] static auto make_rects(glm::ivec2 size, glm::ivec2 tile_count) -> std::vector<Rect<int>>;

	/// \brief Find a tile given its ID.
	/// \returns Pointer to const Tile if found. nullptr if not found.
	[[nodiscard]] auto find_tile(std::string_view id) const -> Ptr<Tile const>;
};
} // namespace bave
