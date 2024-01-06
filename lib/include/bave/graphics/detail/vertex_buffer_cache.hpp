#pragma once
#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace bave::detail {
using VertexBuffer = Buffered<RenderBuffer>;

class VertexBufferCache {
  public:
	explicit VertexBufferCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

	auto allocate() -> std::shared_ptr<VertexBuffer>;

	[[nodiscard]] auto buffer_count() const -> std::size_t;
	auto clear() -> void;

  private:
	Logger m_log{"VertexBufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::vector<std::shared_ptr<VertexBuffer>> m_buffers{};
	mutable std::mutex m_mutex{};
};
} // namespace bave::detail
