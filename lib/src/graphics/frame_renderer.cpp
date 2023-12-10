#include <bave/core/error.hpp>
#include <bave/graphics/frame_renderer.hpp>
#include <bave/graphics/image_barrier.hpp>

namespace bave {
namespace {
auto optimal_depth_format(vk::PhysicalDevice const gpu) -> vk::Format {
	static constexpr auto target{vk::Format::eD32Sfloat};
	auto const props = gpu.getFormatProperties(target);
	if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) { return target; }
	return vk::Format::eD16Unorm;
}

auto make_depth_image(RenderDevice& render_device, vk::Extent2D extent, vk::Format format) {
	auto ici = RenderImage::CreateInfo{
		.format = format,
		.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
		.aspect = vk::ImageAspectFlagBits::eDepth,
		.view_type = vk::ImageViewType::e2D,
		.mip_map = false,
	};
	return std::make_unique<RenderImage>(&render_device, ici, extent);
}

auto make_framebuffer(vk::Device device, vk::RenderPass render_pass, RenderTarget const& target) -> vk::UniqueFramebuffer {
	auto fci = vk::FramebufferCreateInfo{};
	fci.renderPass = render_pass;
	fci.attachmentCount = target.depth ? 2 : 1;
	auto const attachments = std::array{target.colour.view, target.depth};
	fci.pAttachments = attachments.data();
	fci.width = target.colour.extent.width;
	fci.height = target.colour.extent.height;
	fci.layers = 1;
	return device.createFramebufferUnique(fci);
}

auto make_single_render_pass(vk::Device device, vk::Format colour, vk::Format depth) -> vk::UniqueRenderPass {
	auto rpci = vk::RenderPassCreateInfo{};

	auto const attachment_refs = std::array{
		vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal},
		vk::AttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal},
	};
	auto sd = vk::SubpassDescription{};
	sd.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	sd.colorAttachmentCount = 1;
	sd.pColorAttachments = &attachment_refs[0]; // NOLINT (readability-container-data-pointer)
	if (depth != vk::Format{}) { sd.pDepthStencilAttachment = &attachment_refs[1]; }

	auto attachment_descs = std::array<vk::AttachmentDescription, 2>{};
	attachment_descs[0].format = colour;
	attachment_descs[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachment_descs[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachment_descs[0].initialLayout = vk::ImageLayout::eUndefined;
	attachment_descs[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

	attachment_descs[1].format = depth;
	attachment_descs[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachment_descs[1].storeOp = vk::AttachmentStoreOp::eDontCare;
	attachment_descs[1].initialLayout = vk::ImageLayout::eUndefined;
	attachment_descs[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	auto dep = vk::SubpassDependency{};
	dep.srcSubpass = 0;
	dep.dstSubpass = VK_SUBPASS_EXTERNAL;
	dep.srcStageMask = dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dep.srcAccessMask = dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	rpci.pAttachments = attachment_descs.data();
	rpci.attachmentCount = sd.pDepthStencilAttachment == nullptr ? 1 : 2;
	rpci.pSubpasses = &sd;
	rpci.subpassCount = 1;
	rpci.pDependencies = &dep;
	rpci.dependencyCount = 1;

	return device.createRenderPassUnique(rpci);
}
} // namespace

auto FrameRenderer::Frame::make(RenderDevice& render_device) -> Frame {
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
	return ret;
}

FrameRenderer::FrameRenderer(NotNull<RenderDevice*> render_device, NotNull<DataStore const*> data_store)
	: m_render_device(render_device), m_depth_format(optimal_depth_format(render_device->get_gpu().device)), m_blocker(render_device->get_device()) {
	m_frame = Frame::make(*m_render_device);
	m_frame.render_pass = make_single_render_pass(render_device->get_device(), render_device->get_swapchain_format(), m_depth_format);
	m_pipeline_cache = std::make_unique<PipelineCache>(*m_frame.render_pass, render_device, data_store);
}

auto FrameRenderer::start_render(Rgba clear_colour) -> vk::CommandBuffer {
	auto& sync = m_frame.syncs.at(get_frame_index());
	m_frame.swapchain_image = m_render_device->acquire_next_image(*sync.drawn, *sync.draw);
	if (!m_frame.swapchain_image) { return {}; }

	m_pipeline_cache->get_descriptor_cache().next_frame();
	if (!m_frame.depth_image || m_frame.depth_image->get_extent() != m_frame.swapchain_image->extent) {
		if (m_frame.depth_image) { m_render_device->get_defer_queue().push(std::move(m_frame.depth_image)); }
		m_frame.depth_image = make_depth_image(*m_render_device, m_frame.swapchain_image->extent, m_depth_format);
	}

	sync.command_buffer.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

	auto const rt = RenderTarget{.colour = *m_frame.swapchain_image, .depth = m_frame.depth_image->get_image_view()};
	auto& fb = m_frame.framebuffers.at(get_frame_index());
	fb = make_framebuffer(m_render_device->get_device(), *m_frame.render_pass, rt);
	auto const ra = vk::Rect2D{vk::Offset2D{}, m_frame.swapchain_image->extent};

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

auto FrameRenderer::finish_render() -> bool {
	if (!m_frame.swapchain_image) { return false; }

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
	m_render_device->queue_submit(si, *sync.drawn);

	return m_render_device->present_acquired_image(*sync.present);
}

auto FrameRenderer::load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader> {
	if (!is_rendering()) {
		m_log.error("can only load shaders when rendering");
		return {};
	}

	auto& shader_cache = m_pipeline_cache->get_shader_cache();
	auto vert = shader_cache.load(vertex);
	auto frag = shader_cache.load(fragment);
	if (!vert || !frag) { return {}; }

	return Shader{this, vert, frag};
}

auto FrameRenderer::get_backbuffer_extent() const -> vk::Extent2D {
	if (!m_frame.swapchain_image) { return {}; }
	return m_frame.swapchain_image->extent;
}
} // namespace bave
