#pragma once
#include <bave/graphics/buffering.hpp>
#include <bave/graphics/render_resource.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <unordered_map>
#include <vector>

namespace bave {
class ScratchBufferCache {
  public:
	explicit ScratchBufferCache(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

	auto allocate(vk::BufferUsageFlags usage) -> RenderBuffer&;

	auto next_frame() -> void;
	auto clear() -> void;

  private:
	struct Pool {
		std::vector<RenderBuffer> buffers{};
		std::size_t next{};
	};

	using Map = std::unordered_map<vk::BufferUsageFlags, Pool>;

	NotNull<RenderDevice*> m_render_device;
	Buffered<Map> m_maps{};
};
} // namespace bave
