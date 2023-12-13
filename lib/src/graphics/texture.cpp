#include <bave/core/is_positive.hpp>
#include <bave/graphics/image_file.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
namespace {} // namespace

Texture::Texture(NotNull<RenderDevice*> render_device, bool mip_map) : m_image(render_device, RenderImage::CreateInfo{.mip_map = mip_map}) {}

Texture::Texture(NotNull<RenderDevice*> render_device, BitmapView bitmap, bool mip_map)
	: m_image(render_device, RenderImage::CreateInfo{.mip_map = mip_map}, RenderImage::to_vk_extent(bitmap.extent)) {
	m_image.overwrite(bitmap, {});
}

auto Texture::load_image(std::span<std::byte const> compressed, bool mip_map) -> bool {
	auto image_file = ImageFile{};
	if (!image_file.decompress(compressed)) { return false; }

	write(image_file.bitmap(), mip_map);
	return true;
}

void Texture::write(BitmapView bitmap, bool mip_map) {
	if (!is_positive(bitmap.extent)) { return; }

	auto& render_device = m_image.get_render_device();
	render_device.get_defer_queue().push(std::move(m_image));
	m_image = RenderImage{&render_device, RenderImage::CreateInfo{.mip_map = mip_map}, RenderImage::to_vk_extent(bitmap.extent)};
	m_image.overwrite(bitmap, {});
}

auto Texture::combined_image_sampler() const -> ImageSampler {
	auto& sampler_cache = get_image().get_render_device().get_sampler_cache();
	return {.image_view = get_image().get_image_view(), .sampler = sampler_cache.get(sampler)};
}
} // namespace bave
