#include <bave/graphics/detail/texture_image_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
auto TextureImageCache::allocate(RenderImage::CreateInfo const& create_info, vk::Extent2D extent) -> std::shared_ptr<RenderImage> {
	for (auto const& image : m_images) {
		if (image.use_count() == 1) {
			*image = RenderImage{m_render_device, create_info, extent};
			return image;
		}
	}

	m_images.push_back(std::make_shared<RenderImage>(m_render_device, create_info, extent));
	m_log.debug("new Vulkan Texture Image created (total: {})", image_count());

	return m_images.back();
}

auto TextureImageCache::clear() -> void {
	m_render_device->get_device().waitIdle();
	m_log.debug("{} Texture Images destroyed", image_count());
	m_images.clear();
}
} // namespace bave::detail
