#include <bave/graphics/sprite_anim.hpp>
#include <cassert>

namespace bave {
auto SpriteAnim::make_timeline(TextureAtlas const& atlas, Seconds duration) -> AnimTimeline {
	auto const blocks = atlas.get_blocks();
	auto ret = AnimTimeline{.duration = duration};
	ret.tiles.reserve(blocks.size());
	for (auto const& block : blocks) { ret.tiles.emplace_back(block.id); }
	return ret;
}

SpriteAnim::SpriteAnim(std::shared_ptr<TextureAtlas const> atlas, std::shared_ptr<Timeline const> timeline)
	: m_atlas(std::move(atlas)), m_timeline(std::move(timeline)) {
	if (!m_timeline && m_atlas) { m_timeline = std::make_shared<Timeline>(make_timeline(*m_atlas, 1s)); }
	reset_anim();
}

void SpriteAnim::set_texture_atlas(std::shared_ptr<TextureAtlas const> atlas) {
	m_atlas = std::move(atlas);
	reset_anim();
}

void SpriteAnim::set_timeline(std::shared_ptr<Timeline const> timeline) {
	m_timeline = std::move(timeline);
	reset_anim();
}

void SpriteAnim::tick(Seconds dt) {
	if (!animate) {
		elapsed = {};
		return;
	}
	elapsed += dt;
	assert(m_timeline && m_atlas);
	auto const tile_id = m_timeline->get_tile_at(elapsed);
	if (tile_id != m_current_tile_id) {
		if (auto const tile = m_atlas->find_tile(tile_id)) { set_tile(*tile); }
		m_current_tile_id = tile_id;
	}
	if (elapsed > m_timeline->duration) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}

void SpriteAnim::reset_anim() {
	elapsed = {};
	animate = m_atlas && m_timeline;
	m_current_tile_id.clear();

	if (m_timeline) { m_current_tile_id = m_timeline->get_tile_at(elapsed); }
	if (m_atlas) {
		set_texture(m_atlas);
		if (auto const tile = m_atlas->find_tile(m_current_tile_id)) { set_tile(*tile); }
	}
}
} // namespace bave
