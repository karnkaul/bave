#include <bave/graphics/sprite_9slice.hpp>

namespace bave {
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
