#include <bave/core/is_positive.hpp>
#include <bave/graphics/image_file.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
Texture::Texture(NotNull<RenderDevice*> render_device, BitmapView bitmap, bool mip_map)
	: m_render_device(render_device),
	  m_image(render_device->get_image_cache().allocate(detail::RenderImage::CreateInfo{.mip_map = mip_map}, detail::to_vk_extent(bitmap.extent))) {
	if (!bitmap.bytes.empty()) { m_image->overwrite(bitmap, {}); }
}

Texture::~Texture() {
	if (!m_image) { return; }
	m_render_device->get_defer_queue().push(std::move(m_image));
}

auto Texture::get_size() const -> glm::ivec2 {
	if (!m_image) { return {}; }
	auto const extent = m_image->get_extent();
	return glm::ivec2{extent.width, extent.height};
}

auto Texture::get_sampler_image() const -> SamplerImage {
	if (!m_image) { return {}; }
	return {.image_view = m_image->get_image_view(), .sampler = m_render_device->get_sampler_cache().get(sampler)};
}

auto TextureWriteable::load_from_bytes(std::span<std::byte const> compressed) -> bool {
	auto image_file = ImageFile{};
	if (!image_file.load_from_bytes(compressed)) { return false; }

	write(image_file.get_bitmap_view());
	return true;
}

void TextureWriteable::write(BitmapView bitmap) {
	if (!is_positive(bitmap.extent) || !m_image) { return; }
	m_image->recreate(detail::to_vk_extent(bitmap.extent));
	m_image->overwrite(bitmap, {});
}
} // namespace bave
