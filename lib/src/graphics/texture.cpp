#include <bave/graphics/image_file.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
Texture::Texture(NotNull<RenderDevice*> render_device, bool mip_map) : m_image(render_device, RenderImage::CreateInfo{.mip_map = mip_map}) {}

Texture::Texture(NotNull<RenderDevice*> render_device, Bitmap bitmap, bool mip_map)
	: m_image(render_device, RenderImage::CreateInfo{.mip_map = mip_map}, vk::Extent2D{bitmap.extent.x, bitmap.extent.y}) {
	m_image.overwrite(bitmap, {});
}

auto Texture::load_image(std::span<std::byte const> compressed, bool mip_map) -> bool {
	auto image_file = ImageFile{};
	if (!image_file.decompress(compressed)) { return false; }

	write(image_file.bitmap(), mip_map);
	return true;
}

void Texture::write(Bitmap bitmap, bool mip_map) {
	auto& render_device = m_image.get_render_device();
	render_device.get_defer_queue().push(std::move(m_image));
	auto const extent = vk::Extent2D{bitmap.extent.x, bitmap.extent.y};
	m_image = RenderImage{&render_device, RenderImage::CreateInfo{.mip_map = mip_map}, extent};
	m_image.overwrite(bitmap, {});
}

auto Texture::combined_image_sampler() const -> ImageSampler {
	auto& sampler_cache = get_image().get_render_device().get_sampler_cache();
	return {.image_view = get_image().get_image_view(), .sampler = sampler_cache.get(sampler)};
}
} // namespace bave
