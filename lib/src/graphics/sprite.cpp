#include <bave/graphics/sprite.hpp>

namespace bave {
namespace {
auto make_animation(SpriteSheet const& sheet, Seconds const duration) {
	auto const blocks = sheet.get_blocks();
	auto tile_ids = std::vector<std::string>{};
	for (auto const& block : blocks) { tile_ids.push_back(block.id); }
	return SpriteAnimation{std::move(tile_ids), duration};
}
} // namespace

void Sprite::set_size(glm::vec2 const size) {
	if (size != get_size()) {
		auto shape = get_shape();
		shape.size = size;
		set_shape(shape);
	}
}

void Sprite::set_uv(UvRect const uv) {
	if (uv != get_uv()) {
		auto shape = get_shape();
		shape.uv = uv;
		set_shape(shape);
	}
}

void Sprite::set_tile(SpriteSheet::Tile const& tile, bool resize) {
	set_uv(tile.uv);
	if (resize) { set_size(tile.size); }
}

AnimatedSprite::AnimatedSprite(NotNull<RenderDevice*> render_device, NotNull<std::shared_ptr<SpriteSheet>> const& sheet, Seconds const duration)
	: Sprite(render_device), sheet(sheet), animation(make_animation(*sheet, duration)) {
	set_texture(sheet->get_texture());
	m_current_tile_id = animation.get_tile_at({});
	if (auto tile = sheet->find_tile(m_current_tile_id)) { set_tile(*tile); }
}

void AnimatedSprite::tick(Seconds dt) {
	if (!animate) {
		elapsed = {};
		return;
	}
	elapsed += dt;
	auto const tile_id = animation.get_tile_at(elapsed);
	if (tile_id != m_current_tile_id) {
		if (auto tile = sheet->find_tile(tile_id)) { set_tile(*tile); }
		m_current_tile_id = tile_id;
	}
	if (elapsed > animation.get_duration()) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}
} // namespace bave
