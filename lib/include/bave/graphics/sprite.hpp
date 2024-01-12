#pragma once
#include <bave/graphics/shape.hpp>
#include <bave/graphics/sprite_animation.hpp>
#include <bave/graphics/texture_9slice.hpp>
#include <bave/graphics/texture_atlas.hpp>

namespace bave {
class Sprite : public QuadShape {
  public:
	explicit Sprite(NotNull<RenderDevice*> render_device) : QuadShape(render_device) {}

	void set_size(glm::vec2 size);
	void set_auto_size(glm::vec2 max_size);

	void set_uv(UvRect uv);
	void reset_uv() { set_uv(uv_rect_v); }

	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_shape().size; }
	[[nodiscard]] auto get_uv() const -> UvRect const& { return get_shape().uv; }

	void set_tile(TextureAtlas::Tile const& tile);

  protected:
	std::optional<glm::vec2> m_max_size{};
};

class SlicedSprite : public NineQuadShape {
  public:
	explicit SlicedSprite(NotNull<RenderDevice*> render_device) : NineQuadShape(render_device) {}

	void set_texture_9slice(std::shared_ptr<Texture9Slice const> texture);
	void set_size(glm::vec2 size);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_shape().size.current; }
};

class AnimatedSprite : public Sprite {
  public:
	explicit AnimatedSprite(NotNull<RenderDevice*> render_device, std::shared_ptr<TextureAtlas> atlas = {}, Seconds duration = 1s);

	void tick(Seconds dt);

	[[nodiscard]] auto get_current_tile_id() const -> std::string_view { return m_current_tile_id; }

	std::shared_ptr<TextureAtlas> atlas;
	SpriteAnimation animation;
	Seconds elapsed{};
	bool repeat{true};
	bool animate{true};

  private:
	std::string_view m_current_tile_id{};
};
} // namespace bave
