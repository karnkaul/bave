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
		auto const blocks = atlas->get_blocks();
		ret->tiles.reserve(blocks.size());
		for (auto const& block : blocks) { ret->tiles.emplace_back(block.id); }
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

	if (!animate) {
		elapsed = {};
		return;
	}

	assert(m_atlas);
	elapsed += dt;
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
	animate = m_atlas != nullptr;
	m_current_tile_id.clear();

	m_current_tile_id = m_timeline->get_tile_at(elapsed);
	if (m_atlas) {
		if (auto const tile = m_atlas->find_tile(m_current_tile_id)) { set_tile(*tile); }
	}
}
} // namespace bave
