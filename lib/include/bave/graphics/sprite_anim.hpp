#pragma once
#include <bave/graphics/anim_timeline.hpp>
#include <bave/graphics/sprite.hpp>

namespace bave {
class SpriteAnim : public Sprite {
  public:
	using Timeline = AnimTimeline;

	static auto make_timeline(TextureAtlas const& atlas, Seconds duration) -> AnimTimeline;

	explicit SpriteAnim(std::shared_ptr<TextureAtlas const> atlas = {}, std::shared_ptr<Timeline const> timeline = {});

	void set_texture_atlas(std::shared_ptr<TextureAtlas const> atlas);
	void set_timeline(std::shared_ptr<Timeline const> timeline);

	[[nodiscard]] auto get_current_tile_id() const -> std::string_view { return m_current_tile_id; }
	[[nodiscard]] auto get_atlas() const -> std::shared_ptr<TextureAtlas const> const& { return m_atlas; }
	[[nodiscard]] auto get_timeline() const -> std::shared_ptr<Timeline const> const& { return m_timeline; }

	void tick(Seconds dt);

	Seconds elapsed{};
	bool repeat{true};
	bool animate{true};

  private:
	void reset_anim();

	std::shared_ptr<TextureAtlas const> m_atlas;
	std::shared_ptr<Timeline const> m_timeline;
	std::string m_current_tile_id{};
};
} // namespace bave
