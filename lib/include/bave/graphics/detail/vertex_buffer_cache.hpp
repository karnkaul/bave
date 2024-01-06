#pragma once
#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace bave::detail {
class VertexBufferCache {
  public:
	explicit VertexBufferCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

	auto allocate() -> Buffered<std::shared_ptr<RenderBuffer>>;

	[[nodiscard]] auto buffer_count() const -> std::size_t;
	auto clear() -> void;

  private:
	Logger m_log{"VertexBufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::vector<Buffered<std::shared_ptr<RenderBuffer>>> m_buffers{};
	mutable std::mutex m_mutex{};
};
} // namespace bave::detail
