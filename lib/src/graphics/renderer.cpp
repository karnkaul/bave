#include <bave/core/error.hpp>
#include <bave/graphics/detail/image_barrier.hpp>
#include <bave/graphics/renderer.hpp>

namespace bave {
namespace {
auto make_framebuffer(vk::Device device, vk::RenderPass render_pass, RenderTarget const& render_target) -> vk::UniqueFramebuffer {
	auto fci = vk::FramebufferCreateInfo{};
	fci.renderPass = render_pass;
	fci.attachmentCount = 1;
	auto const attachments = std::array{render_target.view};
	fci.pAttachments = attachments.data();
	fci.width = render_target.extent.width;
	fci.height = render_target.extent.height;
	fci.layers = 1;
	return device.createFramebufferUnique(fci);
}

auto make_single_render_pass(vk::Device device, vk::Format colour) -> vk::UniqueRenderPass {
	auto rpci = vk::RenderPassCreateInfo{};

	auto const attachment_refs = std::array{
		vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal},
	};
	auto sd = vk::SubpassDescription{};
	sd.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	sd.colorAttachmentCount = 1;
	sd.pColorAttachments = &attachment_refs[0]; // NOLINT (readability-container-data-pointer)

	auto attachment_descs = std::array<vk::AttachmentDescription, 2>{};
	attachment_descs[0].format = colour;
	attachment_descs[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachment_descs[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachment_descs[0].initialLayout = vk::ImageLayout::eUndefined;
	attachment_descs[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

	auto dep = vk::SubpassDependency{};
	dep.srcSubpass = 0;
	dep.dstSubpass = VK_SUBPASS_EXTERNAL;
	dep.srcStageMask = dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dep.srcAccessMask = dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	rpci.pAttachments = attachment_descs.data();
	rpci.attachmentCount = 1;
	rpci.pSubpasses = &sd;
	rpci.subpassCount = 1;
	rpci.pDependencies = &dep;
	rpci.dependencyCount = 1;

	return device.createRenderPassUnique(rpci);
}

auto white_bitmap() -> BitmapView {
	static constexpr auto pixels = std::array<std::uint8_t, 4>{0xff, 0xff, 0xff, 0xff};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	return BitmapView{.bytes = {reinterpret_cast<std::byte const*>(pixels.data()), pixels.size()}, .extent = {1, 1}};
}
} // namespace

auto Renderer::Frame::make(RenderDevice& render_device) -> Frame {
	auto ret = Frame{};

	auto const device = render_device.get_device();

	for (auto& sync : ret.syncs) {
		sync.command_pool = device.createCommandPoolUnique(vk::CommandPoolCreateInfo{
			vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, render_device.get_gpu().queue_family});
		auto const cbai = vk::CommandBufferAllocateInfo{*sync.command_pool, vk::CommandBufferLevel::ePrimary, 1};
		if (device.allocateCommandBuffers(&cbai, &sync.command_buffer) != vk::Result::eSuccess) { throw Error{"Failed to allocate Vulkan Command Buffer"}; }
		sync.draw = device.createSemaphoreUnique({});
		sync.present = device.createSemaphoreUnique({});
		sync.drawn = device.createFenceUnique({vk::FenceCreateFlagBits::eSignaled});
	}

	ret.render_pass = make_single_render_pass(render_device.get_device(), render_device.get_swapchain_format());

	return ret;
}

Renderer::Renderer(NotNull<RenderDevice*> render_device, NotNull<DataStore const*> data_store)
	: m_render_device(render_device), m_frame(Frame::make(*m_render_device)),
	  m_pipeline_cache(std::make_unique<detail::PipelineCache>(*m_frame.render_pass, render_device, data_store)), m_white(render_device, white_bitmap()),
	  m_blocker(render_device->get_device()) {}

auto Renderer::start_render(Rgba clear_colour) -> vk::CommandBuffer {
	auto& sync = m_frame.syncs.at(get_frame_index());
	m_frame.render_target = m_render_device->acquire_next_image(*sync.drawn, *sync.draw);
	if (!m_frame.render_target) { return {}; }

	m_pipeline_cache->get_descriptor_cache().next_frame();
	sync.command_buffer.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

	auto& fb = m_frame.framebuffers.at(get_frame_index());
	fb = make_framebuffer(m_render_device->get_device(), *m_frame.render_pass, *m_frame.render_target);
	auto const ra = vk::Rect2D{vk::Offset2D{}, m_frame.render_target->extent};

	auto const vec4_clear_colour = clear_colour.to_vec4();
	auto const vk_clear_colour = vk::ClearColorValue{vec4_clear_colour.x, vec4_clear_colour.y, vec4_clear_colour.z, vec4_clear_colour.w};
	auto const clear_values = std::array<vk::ClearValue, 2>{
		vk_clear_colour,
		vk::ClearDepthStencilValue{1.0f, 0},
	};

	auto const rpbi = vk::RenderPassBeginInfo{*m_frame.render_pass, *fb, ra, 2, clear_values.data()};
	sync.command_buffer.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	return sync.command_buffer;
}

auto Renderer::finish_render() -> bool {
	if (!m_frame.render_target) { return false; }

	auto& sync = m_frame.syncs.at(get_frame_index());

	sync.command_buffer.endRenderPass();
	sync.command_buffer.end();

	auto si = vk::SubmitInfo{};
	static constexpr vk::PipelineStageFlags wdsm = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	si.pCommandBuffers = &sync.command_buffer;
	si.commandBufferCount = 1;
	si.pWaitSemaphores = &*sync.draw;
	si.waitSemaphoreCount = 1;
	si.pWaitDstStageMask = &wdsm;
	si.pSignalSemaphores = &*sync.present;
	si.signalSemaphoreCount = 1;

	return m_render_device->submit_and_present(si, *sync.drawn, *sync.present);
}

auto Renderer::get_backbuffer_extent() const -> vk::Extent2D {
	if (!m_frame.render_target) { return {}; }
	return m_frame.render_target->extent;
}
} // namespace bave
