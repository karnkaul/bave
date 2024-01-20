#include <bave/graphics/detail/buffer_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
namespace {
constexpr auto zero_v = std::byte{};

constexpr auto to_usage(BufferType const type) -> vk::BufferUsageFlags {
	switch (type) {
	case BufferType::eVertexIndex: return vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer;
	case BufferType::eStorage: return vk::BufferUsageFlagBits::eStorageBuffer;
	default: return vk::BufferUsageFlagBits::eUniformBuffer;
	}
}

constexpr auto to_str(BufferType const type) -> std::string_view {
	switch (type) {
	case BufferType::eVertexIndex: return "Vertex";
	case BufferType::eUniform: return "Uniform";
	case BufferType::eStorage: return "Storage";
	default: return "unknown";
	}
}

auto make_empty(RenderDevice& render_device) {
	auto ret = std::array{
		RenderBuffer{&render_device, to_usage(BufferType::eVertexIndex)},
		RenderBuffer{&render_device, to_usage(BufferType::eUniform)},
		RenderBuffer{&render_device, to_usage(BufferType::eStorage)},
	};
	for (auto& buffer : ret) { buffer.write(&zero_v, 1); }
	return ret;
}
} // namespace

BufferCache::BufferCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device), m_empty_buffers(make_empty(*render_device)) {}

auto BufferCache::allocate(BufferType const type) -> RenderBuffer& {
	auto const index = static_cast<std::size_t>(type);
	auto& pool = m_maps.at(m_render_device->get_frame_index()).at(index);
	if (pool.next >= pool.buffers.size()) {
		pool.buffers.emplace_back(m_render_device, to_usage(type));
		auto const total = [&] {
			auto ret = std::size_t{};
			for (auto const& pool : m_maps) { ret += pool.at(index).buffers.size(); }
			return ret;
		}();
		m_log.debug("new Vulkan {} Buffer created (total: {})", to_str(type), total);
	}
	return pool.buffers[pool.next++];
}

auto BufferCache::next_frame() -> void {
	for (auto& pool : m_maps.at(m_render_device->get_frame_index())) { pool.next = {}; }
}

auto BufferCache::clear() -> void { m_maps = {}; }
} // namespace bave::detail
