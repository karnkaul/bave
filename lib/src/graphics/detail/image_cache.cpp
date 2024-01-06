#include <bave/graphics/detail/image_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
auto ImageCache::allocate(RenderImage::CreateInfo const& create_info, vk::Extent2D extent) -> std::shared_ptr<RenderImage> {
	auto lock = std::scoped_lock{m_mutex};
	for (auto const& image : m_images) {
		if (image.use_count() == 1) {
			*image = RenderImage{m_render_device, create_info, extent};
			return image;
		}
	}

	m_images.push_back(std::make_shared<RenderImage>(m_render_device, create_info, extent));
	m_log.debug("new Vulkan Texture Image created (total: {})", m_images.size());

	return m_images.back();
}

auto ImageCache::image_count() const -> std::size_t {
	auto lock = std::scoped_lock{m_mutex};
	return m_images.size();
}

auto ImageCache::clear() -> void {
	m_render_device->get_device().waitIdle();
	auto lock = std::scoped_lock{m_mutex};
	m_log.debug("{} Texture Images destroyed", m_images.size());
	m_images.clear();
}
} // namespace bave::detail
