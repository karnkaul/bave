#include <bave/graphics/extent_scaler.hpp>
#include <bave/graphics/sprite.hpp>

namespace bave {
void Sprite::set_size(glm::vec2 const size) {
	if (size != get_size()) {
		auto shape = get_shape();
		shape.size = size;
		set_shape(shape);
	}
	m_max_size.reset();
}

void Sprite::set_auto_size(glm::vec2 const max_size) {
	m_max_size = max_size;
	set_size(*m_max_size);
}

void Sprite::set_uv(UvRect const uv) {
	if (uv != get_uv()) {
		auto shape = get_shape();
		shape.uv = uv;
		set_shape(shape);
	}
}

void Sprite::set_tile(TextureAtlas::Tile const& tile) {
	set_uv(tile.uv);
	if (m_max_size) { set_size(ExtentScaler{.source = tile.size}.fit_space(*m_max_size)); }
}

void Sprite9Slice::set_texture_9slice(std::shared_ptr<Texture9Slice const> texture) {
	if (texture) {
		auto quad = get_shape();
		quad.slice = texture->get_slice();
		quad.size = glm::vec2{texture->get_size()};
		set_shape(quad);
	}
	set_texture(std::move(texture));
}

void Sprite9Slice::set_size(glm::vec2 const size) {
	if (size != get_size()) {
		auto shape = get_shape();
		shape.size.current = size;
		set_shape(shape);
	}
}
} // namespace bave
