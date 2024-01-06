#include <bave/graphics/detail/vertex_buffer_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
namespace {
auto make_buffered(RenderDevice& render_device, vk::BufferUsageFlags usage) -> Buffered<std::shared_ptr<RenderBuffer>> {
	auto ret = Buffered<std::shared_ptr<RenderBuffer>>{};
	fill_buffered(ret, [&render_device, usage] { return std::make_shared<RenderBuffer>(&render_device, usage); });
	return ret;
}
} // namespace

auto VertexBufferCache::allocate() -> Buffered<std::shared_ptr<RenderBuffer>> {
	for (auto const& buffer : m_buffers) {
		if (buffer[0].use_count() == 1) { return buffer; }
	}

	m_buffers.push_back(make_buffered(*m_render_device, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer));
	m_log.debug("new Vulkan Vertex Buffer created (total: {})", m_buffers.size());

	return m_buffers.back();
}

auto VertexBufferCache::clear() -> void {
	m_render_device->get_device().waitIdle();
	m_log.debug("{} Vertex Buffers destroyed", m_buffers.size());
	m_buffers.clear();
}
} // namespace bave::detail
