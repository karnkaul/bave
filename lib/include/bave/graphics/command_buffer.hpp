#pragma once
#include <bave/core/not_null.hpp>
#include <vulkan/vulkan.hpp>

namespace bave {
class RenderDevice;

class CommandBuffer {
  public:
	CommandBuffer(RenderDevice const& render_device);

	[[nodiscard]] auto get() const -> vk::CommandBuffer { return m_cb; }

	auto submit(RenderDevice& render_device) -> void;

	operator vk::CommandBuffer() const { return get(); }

  private:
	vk::UniqueCommandPool m_pool{};
	vk::CommandBuffer m_cb{};
};
} // namespace bave
