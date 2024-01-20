#pragma once
#include <bave/graphics/anim_timeline.hpp>
#include <bave/graphics/sprite.hpp>

namespace bave {
class SpriteAnim : public Sprite {
  public:
	explicit SpriteAnim(std::shared_ptr<TextureAtlas> atlas = {}, Seconds duration = 1s);

	void tick(Seconds dt);

	[[nodiscard]] auto get_current_tile_id() const -> std::string_view { return m_current_tile_id; }

	std::shared_ptr<TextureAtlas> atlas{};
	AnimTimeline timeline{};
	Seconds elapsed{};
	bool repeat{true};
	bool animate{true};

  private:
	std::string m_current_tile_id{};
};
} // namespace bave
