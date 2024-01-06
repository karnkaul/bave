#include <bave/core/is_positive.hpp>
#include <bave/graphics/image_file.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
Texture::Texture(NotNull<RenderDevice*> render_device, bool mip_map)
	: m_image(render_device->get_texture_image_cache().allocate(detail::RenderImage::CreateInfo{.mip_map = mip_map})) {}

Texture::Texture(NotNull<RenderDevice*> render_device, BitmapView bitmap, bool mip_map)
	: m_image(render_device->get_texture_image_cache().allocate(detail::RenderImage::CreateInfo{.mip_map = mip_map}, detail::to_vk_extent(bitmap.extent))) {
	m_image->overwrite(bitmap, {});
}

auto Texture::load_from_bytes(std::span<std::byte const> compressed) -> bool {
	auto image_file = ImageFile{};
	if (!image_file.load_from_bytes(compressed)) { return false; }

	write(image_file.get_bitmap_view());
	return true;
}

void Texture::write(BitmapView bitmap) {
	if (!is_positive(bitmap.extent)) { return; }
	m_image->recreate(detail::to_vk_extent(bitmap.extent));
	m_image->overwrite(bitmap, {});
}

auto Texture::get_size() const -> glm::ivec2 {
	auto const extent = get_image()->get_extent();
	return glm::ivec2{extent.width, extent.height};
}

auto Texture::combined_image_sampler() const -> CombinedImageSampler {
	auto const& image = *get_image();
	return {.image_view = image.get_image_view(), .sampler = image.get_render_device().get_sampler_cache().get(sampler)};
}
} // namespace bave
