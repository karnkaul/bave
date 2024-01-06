#pragma once
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <vector>

namespace bave::detail {
class TextureImageCache {
  public:
	explicit TextureImageCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

	auto allocate(RenderImage::CreateInfo const& create_info, vk::Extent2D extent = {1, 1}) -> std::shared_ptr<RenderImage>;

	[[nodiscard]] auto image_count() const -> std::size_t { return m_images.size(); }
	auto clear() -> void;

  private:
	Logger m_log{"VertexBufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::vector<std::shared_ptr<RenderImage>> m_images{};
};
} // namespace bave::detail
