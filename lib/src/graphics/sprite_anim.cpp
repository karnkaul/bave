#include <bave/graphics/sprite_anim.hpp>
#include <algorithm>

namespace bave {
namespace {
auto make_animation(std::shared_ptr<TextureAtlas> const& atlas, Seconds const duration) {
	if (atlas) {
		auto const blocks = atlas->get_blocks();
		auto tile_ids = std::vector<std::string>{};
		for (auto const& block : blocks) { tile_ids.push_back(block.id); }
		return SpriteAnim::Animation{std::move(tile_ids), duration};
	}
	return SpriteAnim::Animation{std::vector<std::string>{}, {}};
}
} // namespace

auto SpriteAnim::Animation::get_tile_at(Seconds const timestamp) const -> std::string_view {
	if (tiles.empty()) { return {}; }
	if (duration <= 0s) { return tiles.back(); }
	auto const ratio = std::clamp(timestamp / duration, 0.0f, 1.0f);
	auto const index = static_cast<std::size_t>(ratio * static_cast<float>(tiles.size()));
	if (index >= tiles.size()) { return tiles.back(); }
	return tiles.at(index);
}

SpriteAnim::SpriteAnim(NotNull<RenderDevice*> render_device, std::shared_ptr<TextureAtlas> atlas, Seconds const duration)
	: Sprite(render_device), atlas(std::move(atlas)), animation(make_animation(this->atlas, duration)) {
	m_current_tile_id = animation.get_tile_at({});
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
	auto const tile_id = animation.get_tile_at(elapsed);
	if (tile_id != m_current_tile_id) {
		if (auto tile = atlas->find_tile(tile_id)) { set_tile(*tile); }
		m_current_tile_id = tile_id;
	}
	if (elapsed > animation.duration) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}
} // namespace bave
