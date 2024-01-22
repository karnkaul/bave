#include <bave/graphics/sprite_anim.hpp>
#include <cassert>

namespace bave {
namespace {
auto make_timeline(std::shared_ptr<AnimTimeline const> timeline, std::shared_ptr<TextureAtlas const> const& atlas, Seconds duration)
	-> std::shared_ptr<AnimTimeline const> {
	if (timeline) { return timeline; }
	auto ret = std::make_shared<AnimTimeline>();
	ret->duration = duration;
	if (atlas) {
		auto const& tiles = atlas->get_sheet().tiles;
		ret->tiles.reserve(tiles.size());
		for (auto const& block : tiles) { ret->tiles.emplace_back(block.id); }
	}
	return ret;
}
} // namespace

SpriteAnim::SpriteAnim(std::shared_ptr<TextureAtlas const> atlas, std::shared_ptr<Timeline const> timeline)
	: m_timeline(make_timeline(std::move(timeline), atlas, 1s)), m_atlas(std::move(atlas)) {
	reset_anim();
}

void SpriteAnim::set_texture_atlas(std::shared_ptr<TextureAtlas const> atlas) {
	if (!atlas) { return; }
	m_atlas = std::move(atlas);
	reset_anim();
}

void SpriteAnim::set_timeline(std::shared_ptr<Timeline const> timeline) {
	if (!timeline) { return; }
	m_timeline = std::move(timeline);
	reset_anim();
}

void SpriteAnim::tick(Seconds dt) {
	if (textures[0] != m_atlas) { textures[0] = m_atlas; }

	if (!m_atlas) { animate = false; }

	if (!animate) {
		elapsed = {};
		return;
	}

	elapsed += dt;
	auto const tile_id = m_timeline->get_tile_at(elapsed);
	if (tile_id != m_current_tile_id) { set_current_tile(tile_id); }
	if (elapsed > m_timeline->duration) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}

void SpriteAnim::reset_anim() {
	elapsed = {};
	animate = m_atlas != nullptr;
	m_current_tile_id.clear();
	m_current_tile = {};

	if (m_atlas) { set_current_tile(m_timeline->get_tile_at(elapsed)); }
}

auto SpriteAnim::set_current_tile(std::string_view const id) -> bool {
	assert(m_atlas);
	if (auto const* tile = m_atlas->get_sheet().find_tile(id)) {
		set_current_tile(*tile);
		return true;
	}
	return false;
}

void SpriteAnim::set_current_tile(Tile const& tile) {
	set_tile(tile, m_atlas->get_size());
	m_current_tile = &tile;
	m_current_tile_id = tile.id;
}
} // namespace bave
