#pragma once
#include <bave/core/not_null.hpp>
#include <bave/core/string_hash.hpp>
#include <bave/graphics/rect.hpp>
#include <bave/graphics/texture.hpp>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>

namespace bave {
class SpriteSheet {
  public:
	struct Block {
		std::string id{};
		Rect<int> rect{};
	};

	struct Tile {
		glm::ivec2 size{};
		UvRect uv{};
	};

	[[nodiscard]] static auto make_rects(glm::ivec2, glm::ivec2 tile_count) -> std::vector<Rect<int>>;

	explicit SpriteSheet(NotNull<std::shared_ptr<Texture const>> const& texture, std::vector<Block> blocks);

	[[nodiscard]] auto find_tile(std::string_view id) const -> std::optional<Tile>;

	[[nodiscard]] auto get_tile_count() const -> std::size_t { return m_rects.size(); }
	[[nodiscard]] auto get_texture() const -> std::shared_ptr<Texture const> const& { return m_texture.get(); }

	[[nodiscard]] auto get_blocks() const -> std::span<Block const> { return m_blocks; }

  private:
	NotNull<std::shared_ptr<Texture const>> m_texture;
	std::unordered_map<std::string, Rect<int>, StringHash, std::equal_to<>> m_rects{};
	std::vector<Block> m_blocks{};
};
} // namespace bave
