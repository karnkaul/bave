#pragma once
#include <bave/graphics/buffering.hpp>
#include <bave/graphics/render_resource.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <vector>

namespace bave {
class RenderBufferCache {
  public:
	explicit RenderBufferCache(NotNull<RenderDevice*> render_device, vk::BufferUsageFlags usage) : m_render_device(render_device), m_usage(usage) {}

	auto allocate() -> Buffered<std::shared_ptr<RenderBuffer>>;

	[[nodiscard]] auto buffer_count() const -> std::size_t { return m_buffers.size(); }
	auto clear() -> void;

  private:
	Logger m_log{"VertexBufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::vector<Buffered<std::shared_ptr<RenderBuffer>>> m_buffers{};
	vk::BufferUsageFlags m_usage{};
};
} // namespace bave
