#include <bave/core/error.hpp>
#include <bave/graphics/detail/command_buffer.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
CommandBuffer::CommandBuffer(RenderDevice const& render_device)
	: m_pool(render_device.get_device().createCommandPoolUnique({vk::CommandPoolCreateFlagBits::eTransient})) {
	auto const cbai = vk::CommandBufferAllocateInfo{*m_pool, vk::CommandBufferLevel::ePrimary, 1};
	if (render_device.get_device().allocateCommandBuffers(&cbai, &m_cb) != vk::Result::eSuccess) { throw Error{"Failed to allocate Vulkan Command Buffer"}; }
	m_cb.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
}

auto CommandBuffer::submit(RenderDevice& render_device) -> void {
	if (!m_cb) { return; }
	m_cb.end();
	auto vsi = vk::SubmitInfo{};
	vsi.commandBufferCount = 1;
	vsi.pCommandBuffers = &m_cb;
	auto fence = render_device.get_device().createFenceUnique({});
	render_device.queue_submit(vsi, *fence);
	render_device.wait_for(*fence);
	m_cb = vk::CommandBuffer{};
}
} // namespace bave::detail
