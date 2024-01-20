#include <bave/graphics/sprite_anim.hpp>
#include <algorithm>

namespace bave {
namespace {
auto make_timeline(std::shared_ptr<TextureAtlas const> const& atlas, AnimTimeline timeline) -> AnimTimeline {
	if (!atlas || !timeline.tiles.empty()) { return timeline; }
	auto const blocks = atlas->get_blocks();
	timeline.tiles.reserve(blocks.size());
	for (auto const& block : blocks) { timeline.tiles.push_back(block.id); }
	return timeline;
}
} // namespace

SpriteAnim::SpriteAnim(std::shared_ptr<TextureAtlas const> atlas, AnimTimeline timeline) : m_timeline(make_timeline(atlas, std::move(timeline))) {
	set_texture_atlas(std::move(atlas));
}

void SpriteAnim::set_texture_atlas(std::shared_ptr<TextureAtlas const> atlas) {
	m_atlas = std::move(atlas);
	reset_anim();
}

void SpriteAnim::set_timeline(AnimTimeline timeline) {
	m_timeline = std::move(timeline);
	reset_anim();
}

void SpriteAnim::set_duration(Seconds const duration) {
	m_timeline.duration = duration;
	elapsed = {};
}

void SpriteAnim::tick(Seconds dt) {
	if (!animate) {
		elapsed = {};
		return;
	}
	elapsed += dt;
	auto const tile_id = m_timeline.get_tile_at(elapsed);
	if (tile_id != m_current_tile_id) {
		if (auto const tile = m_atlas->find_tile(tile_id)) { set_tile(*tile); }
		m_current_tile_id = tile_id;
	}
	if (elapsed > m_timeline.duration) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}

void SpriteAnim::reset_anim() {
	elapsed = {};
	animate = m_atlas != nullptr;
	m_current_tile_id = m_timeline.get_tile_at(elapsed);
	if (m_atlas) {
		set_texture(m_atlas);
		if (auto tile = m_atlas->find_tile(m_current_tile_id)) { set_tile(*tile); }
	}
}
} // namespace bave
