#pragma once
#include <bave/core/time.hpp>
#include <bave/graphics/sprite.hpp>
#include <string>

namespace bave {
class SpriteAnim : public Sprite {
  public:
	struct Animation {
		std::vector<std::string> tiles{};
		Seconds duration{};

		[[nodiscard]] auto get_tile_at(Seconds timestamp) const -> std::string_view;
	};

	explicit SpriteAnim(std::shared_ptr<TextureAtlas> atlas = {}, Seconds duration = 1s);

	void tick(Seconds dt);

	[[nodiscard]] auto get_current_tile_id() const -> std::string_view { return m_current_tile_id; }

	std::shared_ptr<TextureAtlas> atlas;
	Animation animation;
	Seconds elapsed{};
	bool repeat{true};
	bool animate{true};

  private:
	std::string m_current_tile_id{};
};
} // namespace bave
