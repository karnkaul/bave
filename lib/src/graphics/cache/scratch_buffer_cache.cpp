#include <bave/graphics/cache/scratch_buffer_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave {
auto ScratchBufferCache::allocate(vk::BufferUsageFlags const usage) -> RenderBuffer& {
	auto& pool = m_maps.at(m_render_device->get_frame_index())[usage];
	if (pool.next >= pool.buffers.size()) { pool.buffers.emplace_back(m_render_device, usage); }
	return pool.buffers[pool.next++];
}

auto ScratchBufferCache::next_frame() -> void {
	for (auto& [_, pool] : m_maps.at(m_render_device->get_frame_index())) { pool.next = {}; }
}

auto ScratchBufferCache::clear() -> void { m_maps = {}; }
} // namespace bave
