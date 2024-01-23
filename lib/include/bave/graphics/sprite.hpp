#pragma once
#include <bave/graphics/shape.hpp>
#include <bave/graphics/texture_9slice.hpp>
#include <bave/graphics/texture_atlas.hpp>
#include <bave/graphics/tile_sheet.hpp>
#include <optional>

namespace bave {
class Sprite : public QuadShape {
  public:
	void set_size(glm::vec2 size);
	void set_auto_size(glm::vec2 max_size);

	void set_uv(UvRect uv);
	void reset_uv() { set_uv(uv_rect_v); }

	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_shape().size; }
	[[nodiscard]] auto get_uv() const -> UvRect const& { return get_shape().uv; }

	void set_tile(TileSheet::Tile const& tile);

  protected:
	void set_tile(TileSheet::Tile const& tile, glm::ivec2 total_size);

	std::optional<glm::vec2> m_max_size{};
};

// TODO: move to its own header
class Sprite9Slice : public NineQuadShape {
  public:
	void set_texture_9slice(std::shared_ptr<Texture9Slice const> texture);
	void set_size(glm::vec2 size);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_shape().size.current; }
};
} // namespace bave
