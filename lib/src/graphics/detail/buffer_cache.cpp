#include <bave/core/fixed_string.hpp>
#include <bave/graphics/detail/buffer_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
namespace {
constexpr auto zero_v = std::byte{};

constexpr auto usage_str(vk::BufferUsageFlagBits const bit) -> std::string_view {
	switch (bit) {
	case vk::BufferUsageFlagBits::eVertexBuffer: return "vertex";
	case vk::BufferUsageFlagBits::eIndexBuffer: return "index";
	case vk::BufferUsageFlagBits::eUniformBuffer: return "uniform";
	case vk::BufferUsageFlagBits::eStorageBuffer: return "storage";
	default: return "other";
	}
}

auto make_usage_str(vk::BufferUsageFlags const usage) -> FixedString<64> {
	static constexpr auto bits_v = std::array{
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::BufferUsageFlagBits::eIndexBuffer,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::BufferUsageFlagBits::eStorageBuffer,
	};

	auto str = FixedString<64>{};
	for (auto const bit : bits_v) {
		if (bit & usage) {
			if (!str.empty()) { str.append(FixedString{" | "}); }
			str.append(FixedString{"{}", usage_str(bit)});
		}
	}
	return str;
}
} // namespace

auto BufferCache::allocate(vk::BufferUsageFlags const usage) -> RenderBuffer& {
	auto& pool = m_maps.at(m_render_device->get_frame_index())[usage];
	if (pool.next >= pool.buffers.size()) {
		pool.buffers.emplace_back(m_render_device, usage);
		m_log.info("new Vulkan Buffer created '{}'", make_usage_str(usage).view());
	}
	return pool.buffers[pool.next++];
}

auto BufferCache::get_empty(vk::BufferUsageFlags const usage) -> RenderBuffer const& {
	auto it = m_empty_buffers.find(usage);
	if (it == m_empty_buffers.end()) {
		auto [i, _] = m_empty_buffers.insert_or_assign(usage, RenderBuffer{m_render_device, usage});
		i->second.write(&zero_v, 1);
		it = i;
	}
	return it->second;
}

auto BufferCache::next_frame() -> void {
	for (auto& [_, pool] : m_maps.at(m_render_device->get_frame_index())) { pool.next = {}; }
}

auto BufferCache::clear() -> void { m_maps = {}; }
} // namespace bave::detail
