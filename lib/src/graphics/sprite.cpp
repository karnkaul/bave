#include <bave/graphics/sprite.hpp>

namespace bave {
namespace {
auto make_animation(std::shared_ptr<TiledTexture> const& sheet, Seconds const duration) {
	if (sheet) {
		auto const blocks = sheet->get_blocks();
		auto tile_ids = std::vector<std::string>{};
		for (auto const& block : blocks) { tile_ids.push_back(block.id); }
		return SpriteAnimation{std::move(tile_ids), duration};
	}
	return SpriteAnimation{std::vector<std::string>{}, {}};
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

void Sprite::set_tile(TiledTexture::Tile const& tile, bool resize) {
	set_uv(tile.uv);
	if (resize) { set_size(tile.size); }
}

void SlicedSprite::set_sliced_texture(std::shared_ptr<SlicedTexture const> texture) {
	if (texture) {
		auto quad = get_shape();
		quad.slice = texture->get_slice();
		quad.size = glm::vec2{texture->get_size()};
		set_shape(quad);
	}
	set_texture(std::move(texture));
}

void SlicedSprite::set_size(glm::vec2 const size) {
	if (size != get_size()) {
		auto shape = get_shape();
		shape.size.current = size;
		set_shape(shape);
	}
}

AnimatedSprite::AnimatedSprite(NotNull<RenderDevice*> render_device, std::shared_ptr<TiledTexture> sheet, Seconds const duration)
	: Sprite(render_device), sheet(std::move(sheet)), animation(make_animation(this->sheet, duration)) {
	m_current_tile_id = animation.get_tile_at({});
	if (this->sheet) {
		set_texture(this->sheet);
		if (auto tile = this->sheet->find_tile(m_current_tile_id)) { set_tile(*tile); }
	}
}

void AnimatedSprite::tick(Seconds dt) {
	if (!sheet) { animate = false; }
	set_texture(sheet);
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
	if (elapsed > animation.duration) {
		elapsed = {};
		if (!repeat) { animate = false; }
	}
}
} // namespace bave
