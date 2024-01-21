#include <bave/graphics/sprite_anim.hpp>
#include <algorithm>

namespace bave {
auto SpriteAnim::make_timeline(TextureAtlas const& atlas, Seconds duration) -> AnimTimeline {
	auto const blocks = atlas.get_blocks();
	auto ret = AnimTimeline{.duration = duration};
	ret.keyframes.reserve(blocks.size());
	for (auto const& block : blocks) { ret.keyframes.push_back(AnimKeyframe{.tile = block.id}); }
	return ret;
}

SpriteAnim::SpriteAnim(std::shared_ptr<TextureAtlas const> atlas, Timeline timeline) : m_timeline(timeline) { set_texture_atlas(std::move(atlas)); }

void SpriteAnim::set_texture_atlas(std::shared_ptr<TextureAtlas const> atlas) {
	m_atlas = std::move(atlas);
	reset_anim();
}

void SpriteAnim::set_timeline(Timeline timeline) {
	m_timeline = timeline;
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
	auto const& keyframe = m_timeline.get_keyframe_at(elapsed);
	if (keyframe.tile != m_current_tile_id) {
		if (auto const tile = m_atlas->find_tile(keyframe.tile)) { set_tile(*tile); }
		m_current_tile_id = keyframe.tile;
	}
	if (elapsed > m_timeline.duration) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}

void SpriteAnim::reset_anim() {
	elapsed = {};
	animate = m_atlas != nullptr;
	m_current_tile_id = m_timeline.get_keyframe_at(elapsed).tile;
	if (m_atlas) {
		set_texture(m_atlas);
		if (auto tile = m_atlas->find_tile(m_current_tile_id)) { set_tile(*tile); }
	}
}
} // namespace bave
