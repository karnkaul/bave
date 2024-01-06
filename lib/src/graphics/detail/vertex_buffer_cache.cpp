#include <bave/graphics/detail/vertex_buffer_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
namespace {
auto make_vertex_buffer(RenderDevice& render_device) -> std::shared_ptr<VertexBuffer> {
	auto const factory = [&render_device] {
		return RenderBuffer{&render_device, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer};
	};
	return std::make_shared<VertexBuffer>(make_buffered<RenderBuffer>(factory));
}
} // namespace

auto VertexBufferCache::allocate() -> std::shared_ptr<VertexBuffer> {
	auto lock = std::scoped_lock{m_mutex};
	for (auto const& buffer : m_buffers) {
		if (buffer.use_count() == 1) { return buffer; }
	}

	m_buffers.push_back(make_vertex_buffer(*m_render_device));
	m_log.debug("new Vulkan Vertex Buffer created (total: {})", m_buffers.size());

	return m_buffers.back();
}

auto VertexBufferCache::buffer_count() const -> std::size_t {
	auto lock = std::scoped_lock{m_mutex};
	return m_buffers.size();
}

auto VertexBufferCache::clear() -> void {
	m_render_device->get_device().waitIdle();
	auto lock = std::scoped_lock{m_mutex};
	m_log.debug("{} Vertex Buffers destroyed", m_buffers.size());
	m_buffers.clear();
}
} // namespace bave::detail
