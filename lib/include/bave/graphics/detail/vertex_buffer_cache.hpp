#pragma once
#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <vector>

namespace bave::detail {
class VertexBufferCache {
  public:
	explicit VertexBufferCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

	auto allocate() -> Buffered<std::shared_ptr<RenderBuffer>>;

	[[nodiscard]] auto buffer_count() const -> std::size_t { return m_buffers.size(); }
	auto clear() -> void;

  private:
	Logger m_log{"VertexBufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::vector<Buffered<std::shared_ptr<RenderBuffer>>> m_buffers{};
};
} // namespace bave::detail
