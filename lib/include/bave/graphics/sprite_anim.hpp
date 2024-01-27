#pragma once
#include <bave/graphics/anim_timeline.hpp>
#include <bave/graphics/sprite.hpp>

namespace bave {
/// \brief Animated sprite.
class SpriteAnim : public Sprite {
  public:
	using Timeline = AnimTimeline;
	using Tile = TileSheet::Tile;

	/// \brief Constructor.
	/// \param atlas Texture atlas.
	/// \param timeline Animation timeline.
	explicit SpriteAnim(std::shared_ptr<TextureAtlas const> atlas = {}, std::shared_ptr<Timeline const> timeline = {});

	/// \brief Set the texture atlas. Replaces primary texture.
	void set_texture_atlas(std::shared_ptr<TextureAtlas const> atlas);
	/// \brief Set the animation timeline.
	void set_timeline(std::shared_ptr<Timeline const> timeline);

	[[nodiscard]] auto get_current_tile() const -> Ptr<Tile const> { return m_current_tile; }
	[[nodiscard]] auto get_current_tile_id() const -> std::string_view { return m_current_tile_id; }
	[[nodiscard]] auto get_timeline() const -> NotNull<std::shared_ptr<Timeline const>> const& { return m_timeline; }
	[[nodiscard]] auto get_atlas() const -> std::shared_ptr<TextureAtlas const> const& { return m_atlas; }

	/// \brief Update animation dt seconds forward.
	/// \param dt Duration to add to elapsed.
	void tick(Seconds dt);

	/// \brief Current timestamp.
	Seconds elapsed{};
	/// \brief Whether to loop-back and repeat.
	bool repeat{true};
	/// \brief Whether to animate.
	bool animate{true};

  private:
	auto set_current_tile(std::string_view id) -> bool;
	void set_current_tile(Tile const& tile);
	void reset_anim();

	NotNull<std::shared_ptr<Timeline const>> m_timeline;
	std::shared_ptr<TextureAtlas const> m_atlas;
	std::string m_current_tile_id{};
	Ptr<Tile const> m_current_tile{};
};
} // namespace bave
