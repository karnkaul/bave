#pragma once
#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/logger.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <unordered_map>
#include <vector>

namespace bave::detail {
class BufferCache {
  public:
	explicit BufferCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

	auto allocate(vk::BufferUsageFlags usage) -> RenderBuffer&;
	auto get_empty(vk::BufferUsageFlags usage) -> RenderBuffer const&;
	auto or_empty(Ptr<RenderBuffer const> buffer, vk::BufferUsageFlags usage) -> RenderBuffer const& { return buffer == nullptr ? get_empty(usage) : *buffer; }

	auto next_frame() -> void;
	auto clear() -> void;

  private:
	struct Pool {
		std::vector<RenderBuffer> buffers{};
		std::size_t next{};
	};

	using Map = std::unordered_map<vk::BufferUsageFlags, Pool>;

	Logger m_log{"BufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::unordered_map<vk::BufferUsageFlags, RenderBuffer> m_empty_buffers{};
	Buffered<Map> m_maps{};
};
} // namespace bave::detail
