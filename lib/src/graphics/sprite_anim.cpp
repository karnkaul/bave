#include <bave/graphics/sprite_anim.hpp>
#include <algorithm>

namespace bave {
namespace {
auto make_timeline(std::shared_ptr<TextureAtlas> const& atlas, Seconds const duration) -> AnimTimeline {
	if (!atlas) { return {}; }
	auto const blocks = atlas->get_blocks();
	auto tile_ids = std::vector<std::string>{};
	for (auto const& block : blocks) { tile_ids.push_back(block.id); }
	return AnimTimeline{std::move(tile_ids), duration};
}
} // namespace

SpriteAnim::SpriteAnim(std::shared_ptr<TextureAtlas> atlas, Seconds const duration) : atlas(std::move(atlas)), timeline(make_timeline(this->atlas, duration)) {
	m_current_tile_id = timeline.get_tile_at({});
	if (this->atlas) {
		set_texture(this->atlas);
		if (auto tile = this->atlas->find_tile(m_current_tile_id)) { set_tile(*tile); }
	}
}

void SpriteAnim::tick(Seconds dt) {
	if (!atlas) { animate = false; }
	set_texture(atlas);
	if (!animate) {
		elapsed = {};
		return;
	}
	elapsed += dt;
	auto const tile_id = timeline.get_tile_at(elapsed);
	if (tile_id != m_current_tile_id) {
		if (auto tile = atlas->find_tile(tile_id)) { set_tile(*tile); }
		m_current_tile_id = tile_id;
	}
	if (elapsed > timeline.duration) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}
} // namespace bave
