#include <bave/graphics/detail/scratch_buffer_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
namespace {
constexpr auto zero_v = std::byte{};
}

auto ScratchBufferCache::allocate(vk::BufferUsageFlags const usage) -> RenderBuffer& {
	auto& pool = m_maps.at(m_render_device->get_frame_index())[usage];
	if (pool.next >= pool.buffers.size()) { pool.buffers.emplace_back(m_render_device, usage); }
	return pool.buffers[pool.next++];
}

auto ScratchBufferCache::get_empty(vk::BufferUsageFlags const usage) -> RenderBuffer const& {
	auto it = m_empty_buffers.find(usage);
	if (it == m_empty_buffers.end()) {
		auto [i, _] = m_empty_buffers.insert_or_assign(usage, RenderBuffer{m_render_device, usage});
		i->second.write(&zero_v, 1);
		it = i;
	}
	return it->second;
}

auto ScratchBufferCache::next_frame() -> void {
	for (auto& [_, pool] : m_maps.at(m_render_device->get_frame_index())) { pool.next = {}; }
}

auto ScratchBufferCache::clear() -> void { m_maps = {}; }
} // namespace bave::detail
