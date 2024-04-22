#include <bave/core/error.hpp>
#include <bave/core/visitor.hpp>
#include <bave/graphics/detail/image_barrier.hpp>
#include <bave/graphics/renderer.hpp>

namespace bave {
namespace {
auto make_framebuffer(vk::Device device, vk::RenderPass render_pass, detail::RenderTarget const& render_target) -> vk::UniqueFramebuffer {
	auto fci = vk::FramebufferCreateInfo{};
	fci.renderPass = render_pass;
	auto attachments = std::array<vk::ImageView, 2>{};
	if (render_target.msaa) {
		attachments = {render_target.msaa, render_target.swapchain};
	} else {
		attachments[0] = render_target.swapchain;
	}
	fci.pAttachments = attachments.data();
	fci.attachmentCount = render_target.msaa ? 2 : 1;
	fci.width = render_target.extent.width;
	fci.height = render_target.extent.height;
	fci.layers = 1;
	return device.createFramebufferUnique(fci);
}

auto make_single_render_pass(vk::Device device, vk::Format colour, vk::SampleCountFlagBits samples) -> vk::UniqueRenderPass {
	auto rpci = vk::RenderPassCreateInfo{};

	auto const attachment_refs = std::array{
		vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal},
		vk::AttachmentReference{1, vk::ImageLayout::eColorAttachmentOptimal},
	};
	auto sd = vk::SubpassDescription{};
	sd.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	sd.colorAttachmentCount = 1;
	sd.pColorAttachments = &attachment_refs[0]; // NOLINT (readability-container-data-pointer)
	if (samples > vk::SampleCountFlagBits::e1) { sd.pResolveAttachments = &attachment_refs[1]; }

	auto attachment_descs = std::array<vk::AttachmentDescription, 2>{};
	attachment_descs[0].format = colour;
	attachment_descs[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachment_descs[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachment_descs[0].initialLayout = vk::ImageLayout::eUndefined;
	attachment_descs[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	attachment_descs[0].samples = samples;

	if (samples > vk::SampleCountFlagBits::e1) {
		attachment_descs[0].storeOp = vk::AttachmentStoreOp::eDontCare;
		attachment_descs[1].format = colour;
		attachment_descs[1].loadOp = vk::AttachmentLoadOp::eClear;
		attachment_descs[1].storeOp = vk::AttachmentStoreOp::eStore;
		attachment_descs[1].initialLayout = vk::ImageLayout::eUndefined;
		attachment_descs[1].finalLayout = vk::ImageLayout::ePresentSrcKHR;
		attachment_descs[1].samples = vk::SampleCountFlagBits::e1;
	}

	auto dep = vk::SubpassDependency{};
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;
	dep.srcStageMask = dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dep.srcAccessMask = dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	rpci.pAttachments = attachment_descs.data();
	rpci.attachmentCount = samples == vk::SampleCountFlagBits::e1 ? 1 : 2;
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

	ret.make_syncs(device, render_device.get_gpu().queue_family);

	ret.render_pass = make_single_render_pass(render_device.get_device(), render_device.get_swapchain_format(), render_device.get_sample_count());

	if (render_device.get_sample_count() > vk::SampleCountFlagBits::e1) {
		auto const ici = detail::RenderImage::CreateInfo{
			.format = render_device.get_swapchain_format(),
			.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
			.aspect = vk::ImageAspectFlagBits::eColor,
			.layout = vk::ImageLayout::eColorAttachmentOptimal,
			.samples = render_device.get_sample_count(),
			.mip_map = false,
			.lazily_allocated = true,
		};
		ret.msaa_image.emplace(&render_device, ici);
	}

	return ret;
}

void Renderer::Frame::make_syncs(vk::Device device, std::uint32_t queue_family) {
	for (auto& sync : syncs) {
		sync.command_pool = device.createCommandPoolUnique(
			vk::CommandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queue_family});
		auto const cbai = vk::CommandBufferAllocateInfo{*sync.command_pool, vk::CommandBufferLevel::ePrimary, 1};
		if (device.allocateCommandBuffers(&cbai, &sync.command_buffer) != vk::Result::eSuccess) { throw Error{"Failed to allocate Vulkan Command Buffer"}; }
		sync.draw = device.createSemaphoreUnique({});
		sync.present = device.createSemaphoreUnique({});
		sync.drawn = device.createFenceUnique({vk::FenceCreateFlagBits::eSignaled});
	}
	render_target.reset();
}

Renderer::Renderer(NotNull<RenderDevice*> render_device, NotNull<detail::DataStoreProvider const*> data_store_provider)
	: m_render_device(render_device), m_frame(Frame::make(*m_render_device)),
	  m_pipeline_cache(std::make_unique<detail::PipelineCache>(*m_frame.render_pass, render_device, data_store_provider)),
	  m_white(render_device, white_bitmap()), m_blocker(render_device->get_device()) {}

auto Renderer::start_render(Rgba const clear_colour) -> bool {
	auto& sync = m_frame.syncs.at(get_frame_index());
	auto const acquire_result = m_render_device->acquire_next_image(*sync.drawn, *sync.draw);
	auto const visitor = Visitor{
		[this](std::monostate) { m_frame.render_target.reset(); },
		[this](detail::RenderTarget render_target) { m_frame.render_target = render_target; },
		[this](RenderDevice::RecreateSync) {
			m_render_device->get_device().waitIdle();
			m_frame.make_syncs(m_render_device->get_device(), m_render_device->get_gpu().queue_family);
		},
	};
	std::visit(visitor, acquire_result);
	if (!m_frame.render_target) { return {}; }

	if (m_frame.msaa_image) {
		m_frame.msaa_image->recreate(m_frame.render_target->extent);
		m_frame.render_target->msaa = m_frame.msaa_image->get_image_view();
	}

	m_pipeline_cache->get_descriptor_cache().next_frame();
	sync.command_buffer.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

	auto& fb = m_frame.framebuffers.at(get_frame_index());
	fb = make_framebuffer(m_render_device->get_device(), *m_frame.render_pass, *m_frame.render_target);
	auto const ra = vk::Rect2D{vk::Offset2D{}, m_frame.render_target->extent};

	auto const vec4_clear_colour = Rgba::to_linear(clear_colour.to_vec4());
	auto const vk_clear_colour = vk::ClearColorValue{vec4_clear_colour.x, vec4_clear_colour.y, vec4_clear_colour.z, vec4_clear_colour.w};
	auto const clear_values = std::array<vk::ClearValue, 2>{
		vk_clear_colour,
		vk::ClearDepthStencilValue{1.0f, 0},
	};

	auto const rpbi = vk::RenderPassBeginInfo{*m_frame.render_pass, *fb, ra, 2, clear_values.data()};
	sync.command_buffer.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	return true;
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

auto Renderer::get_command_buffer() const -> vk::CommandBuffer {
	if (!m_frame.render_target) { return {}; }
	return m_frame.syncs.at(get_frame_index()).command_buffer;
}
} // namespace bave
