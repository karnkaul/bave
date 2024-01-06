#pragma once
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace bave::detail {
class ImageCache {
  public:
	explicit ImageCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

	auto allocate(RenderImage::CreateInfo const& create_info, vk::Extent2D extent = {1, 1}) -> std::shared_ptr<RenderImage>;

	[[nodiscard]] auto image_count() const -> std::size_t;
	auto clear() -> void;

  private:
	Logger m_log{"VertexBufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::vector<std::shared_ptr<RenderImage>> m_images{};
	mutable std::mutex m_mutex{};
};
} // namespace bave::detail
