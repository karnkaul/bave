#include <bave/build_version.hpp>
#include <bave/core/error.hpp>
#include <bave/graphics/detail/utils.hpp>
#include <bave/graphics/projector.hpp>
#include <bave/graphics/render_device.hpp>
#include <src/graphics/detail/bootstrap.hpp>

namespace bave {
namespace {
auto make_vma(vk::Instance instance, vk::PhysicalDevice physical_device, vk::Device device) -> VmaAllocator {
	auto vaci = VmaAllocatorCreateInfo{};
	vaci.instance = instance;
	vaci.physicalDevice = physical_device;
	vaci.device = device;
	auto dl = VULKAN_HPP_DEFAULT_DISPATCHER;
	auto vkFunc = VmaVulkanFunctions{};
	vkFunc.vkGetInstanceProcAddr = dl.vkGetInstanceProcAddr;
	vkFunc.vkGetDeviceProcAddr = dl.vkGetDeviceProcAddr;
	vaci.pVulkanFunctions = &vkFunc;
	auto* ret = VmaAllocator{};
	if (vmaCreateAllocator(&vaci, &ret) != VK_SUCCESS) { throw Error{"Failed to create Vulkan Allocator"}; }
	return ret;
}

constexpr auto image_count(vk::SurfaceCapabilitiesKHR const& caps) noexcept -> std::uint32_t {
	if (caps.maxImageCount < caps.minImageCount) { return std::max(3u, caps.minImageCount); }
	return std::clamp(3u, caps.minImageCount, caps.maxImageCount);
}

constexpr auto image_extent(vk::SurfaceCapabilitiesKHR const& caps, vk::Extent2D const fb) noexcept -> vk::Extent2D {
	constexpr auto limitless_v = std::numeric_limits<std::uint32_t>::max();
	if (caps.currentExtent.width < limitless_v && caps.currentExtent.height < limitless_v) { return caps.currentExtent; }
	auto const x = std::clamp(fb.width, caps.minImageExtent.width, caps.maxImageExtent.width);
	auto const y = std::clamp(fb.height, caps.minImageExtent.height, caps.maxImageExtent.height);
	return vk::Extent2D{x, y};
}

constexpr auto composite_alpha(vk::SurfaceCapabilitiesKHR const& caps) -> vk::CompositeAlphaFlagBitsKHR {
	using enum vk::CompositeAlphaFlagBitsKHR;
	if (caps.supportedCompositeAlpha & eOpaque) { return eOpaque; }
	if (caps.supportedCompositeAlpha & eInherit) { return eInherit; }
	if (caps.supportedCompositeAlpha & ePreMultiplied) { return ePreMultiplied; }
	// according to the spec, at least one bit must be set
	return ePostMultiplied;
}

constexpr auto sample_count(vk::SampleCountFlags const supported, vk::SampleCountFlagBits const desired) {
	if (supported & desired) { return desired; }
	using enum vk::SampleCountFlagBits;
	constexpr auto values_v = std::array{
		e64, e32, e16, e8, e4, e2,
	};
	for (auto const value : values_v) {
		if (desired >= value && (supported & value)) { return value; }
	}
	return e1;
}
} // namespace

RenderDevice::RenderDevice(NotNull<detail::IWsi*> wsi, CreateInfo create_info) : m_wsi(wsi) {
	auto instance_builder = detail::InstanceBuilder{.extensions = wsi->get_instance_extensions(), .validation = create_info.validation_layers};
	auto instance = instance_builder.build();
	if (!instance.instance) { throw Error{"Failed to create Vulkan Instance"}; }
	m_instance = std::move(instance.instance);
	if (instance.is_validation_enabled()) {
		m_debug_messenger = std::move(instance.debug_messenger);
		m_log.info("Vulkan Validation Layers loaded");
	}

	m_surface = vk::UniqueSurfaceKHR{wsi->make_surface(*m_instance), *m_instance};
	if (!m_surface) { throw Error{"Failed to create Vulkan Surface"}; }

	auto device_builder = detail::DeviceBuilder{*m_instance, *m_surface};
	m_gpu = m_wsi->select_gpu(device_builder.get_gpus());
	m_samples = sample_count(m_gpu.device.getProperties().limits.sampledImageColorSampleCounts, create_info.desired_samples);

	auto device = device_builder.build();
	if (!device.device) { throw Error{"Failed to create Vulkan Device"}; }
	m_device = std::move(device.device);
	m_queue = device.queue;

	m_allocator = {make_vma(get_instance(), get_gpu().device, get_device())};

	m_swapchain.present_modes = get_gpu().device.getSurfacePresentModesKHR(get_surface());
	m_swapchain.formats = detail::Swapchain::Formats::make(get_gpu().device.getSurfaceFormatsKHR(get_surface()));
	m_swapchain.make_create_info(*m_surface, m_gpu.queue_family, create_info.swapchain_colour_space);
	m_swapchain.create_info.compositeAlpha = composite_alpha(get_gpu().device.getSurfaceCapabilitiesKHR(get_surface()));

	recreate_swapchain(wsi->get_framebuffer_extent());

	m_buffer_cache = std::make_unique<detail::BufferCache>(this);
	m_image_cache = std::make_unique<detail::ImageCache>(this);
	m_sampler_cache = std::make_unique<detail::SamplerCache>(get_device());

	auto const line_width_range = get_gpu().device.getProperties().limits.lineWidthRange;
	m_line_width_limits = {line_width_range[0], line_width_range[1]};

	render_view = get_default_view();

	m_log.info("using MSAA: {}x", static_cast<int>(m_samples));
}

auto RenderDevice::get_default_view() const -> RenderView {
	auto const extent = get_swapchain_extent();
	return RenderView{.viewport = glm::vec2{extent.width, extent.height}};
}

auto RenderDevice::get_viewport_scaler() const -> ExtentScaler {
	auto const extent = get_swapchain_extent();
	return ExtentScaler{.source = glm::vec2{extent.width, extent.height}};
}

auto RenderDevice::project_to(glm::vec2 const target_space, glm::vec2 const point) const -> glm::vec2 {
	auto const extent = get_swapchain_extent();
	return Projector{.source = glm::vec2{extent.width, extent.height}, .target = target_space}.project(point);
}

auto RenderDevice::wait_for(vk::Fence const fence, std::uint64_t const timeout) const -> bool {
	return get_device().waitForFences(fence, vk::True, timeout) == vk::Result::eSuccess;
}

auto RenderDevice::reset_fence(vk::Fence const fence, bool wait_first) const -> bool {
	if (wait_first && !wait_for(fence)) { return false; }
	get_device().resetFences(fence);
	return true;
}

auto RenderDevice::request_present_mode(vk::PresentModeKHR present_mode) -> bool {
	if (std::find(m_swapchain.present_modes.begin(), m_swapchain.present_modes.end(), present_mode) == m_swapchain.present_modes.end()) {
		m_log.warn("unsupported present mode: '{}'", to_vsync_string(present_mode));
		return false;
	}
	m_swapchain.desired_present_mode = present_mode;
	return true;
}

auto RenderDevice::acquire_next_image(vk::Fence wait, vk::Semaphore signal) -> std::optional<detail::RenderTarget> {
	auto const framebuffer = m_wsi->get_framebuffer_extent();
	if (framebuffer.width == 0 || framebuffer.height == 0) { return {}; }

	reset_fence(wait);
	m_defer_queue.next_frame();

	if (m_swapchain.desired_present_mode != m_swapchain.create_info.presentMode || framebuffer != m_swapchain.create_info.imageExtent) {
		if (!recreate_swapchain(framebuffer)) { return {}; }
	}

	if (!m_swapchain.active.image_index) {
		auto image_index = std::uint32_t{};
		auto lock = std::scoped_lock{m_queue_mutex};
		auto const result = get_device().acquireNextImageKHR(*m_swapchain.active.swapchain, RenderDevice::max_timeout_v, signal, {}, &image_index);
		if (!handle_swapchain_result(result, framebuffer, "acquire_next_image")) { return {}; }

		m_swapchain.active.image_index = image_index;
	}

	assert(m_swapchain.active.image_index.has_value());
	return m_swapchain.active.render_targets.at(*m_swapchain.active.image_index);
}

auto RenderDevice::queue_submit(vk::SubmitInfo const& submit_info, vk::Fence const signal) -> bool {
	auto lock = std::scoped_lock{m_queue_mutex};
	return get_queue().submit(1, &submit_info, signal) == vk::Result::eSuccess;
}

auto RenderDevice::submit_and_present(vk::SubmitInfo const& submit_info, vk::Fence draw_signal, vk::Semaphore present_wait) -> bool {
	auto const framebuffer = m_wsi->get_framebuffer_extent();
	if (framebuffer.width == 0 || framebuffer.height == 0) { return false; }
	if (!m_swapchain.active.image_index) {
		m_log.error("Attempt to present without acquired image");
		return false;
	}

	auto pi = vk::PresentInfoKHR{};
	pi.pImageIndices = &*m_swapchain.active.image_index;
	pi.pSwapchains = &*m_swapchain.active.swapchain;
	pi.pWaitSemaphores = &present_wait;
	pi.waitSemaphoreCount = 1;
	pi.pSwapchains = &*m_swapchain.active.swapchain;
	pi.swapchainCount = 1;

	auto lock = std::unique_lock{m_queue_mutex};
	auto result = get_queue().submit(1, &submit_info, draw_signal);
	result = get_queue().presentKHR(&pi);
	lock.unlock();

	m_frame_index.increment();

	m_swapchain.active.image_index.reset();
	m_buffer_cache->next_frame();

	return handle_swapchain_result(result, framebuffer, "present");
}

auto RenderDevice::recreate_surface() -> bool {
	if (m_swapchain.active.image_index.has_value()) {
		m_log.error("cannot recreate surface in the middle of a render pass");
		return false;
	}

	m_log.info("recreating Vulkan Surface and Swapchain...");
	get_device().waitIdle();
	m_swapchain.active.swapchain.reset();
	m_surface = vk::UniqueSurfaceKHR{m_wsi->make_surface(*m_instance), *m_instance};
	recreate_swapchain(m_wsi->get_framebuffer_extent());
	return true;
}

auto RenderDevice::recreate_swapchain(vk::Extent2D framebuffer) -> bool {
	if (framebuffer.width <= 0 || framebuffer.height <= 0) { return false; }

	auto info = m_swapchain.create_info;
	info.surface = *m_surface;
	auto const caps = get_gpu().device.getSurfaceCapabilitiesKHR(get_surface());
	info.imageExtent = image_extent(caps, framebuffer);
	info.presentMode = m_swapchain.desired_present_mode;
	info.minImageCount = image_count(caps);
	info.oldSwapchain = m_swapchain.active.swapchain.get();
	get_device().waitIdle();
	auto new_swapchain = get_device().createSwapchainKHRUnique(info);

	auto count = std::uint32_t{};
	if (get_device().getSwapchainImagesKHR(*new_swapchain, &count, nullptr) != vk::Result::eSuccess) {
		m_log.error("Failed to get Swapchain Images");
		return false;
	}

	m_swapchain.active.swapchain = std::move(new_swapchain);
	m_swapchain.create_info = info;
	m_swapchain.active.render_targets.resize(count);
	auto const images = get_device().getSwapchainImagesKHR(*m_swapchain.active.swapchain);
	m_swapchain.active.render_targets.clear();
	m_swapchain.active.views.clear();
	for (auto const image : images) {
		m_swapchain.active.views.push_back(detail::MakeImageView{.image = image, .format = m_swapchain.create_info.imageFormat}(get_device()));
		m_swapchain.active.render_targets.push_back({
			.swapchain = *m_swapchain.active.views.back(),
			.extent = m_swapchain.create_info.imageExtent,
			.format = m_swapchain.create_info.imageFormat,
		});
	}

	m_swapchain.active.image_index.reset();

	m_log.info("swapchain extent: [{}x{}] | images: [{}] | colour space: [{}] | vsync: [{}]", m_swapchain.create_info.imageExtent.width,
			   m_swapchain.create_info.imageExtent.height, m_swapchain.active.render_targets.size(),
			   detail::Swapchain::is_srgb_format(info.imageFormat) ? "sRGB" : "linear", to_vsync_string(info.presentMode));

	return true;
}

auto RenderDevice::handle_swapchain_result(vk::Result const result, vk::Extent2D framebuffer, std::string_view const op) -> bool {
	switch (result) {
	case vk::Result::eSuccess: return true;
	// TODO: check suboptimal against Android auto-surface transform
	case vk::Result::eSuboptimalKHR: return true;
	case vk::Result::eErrorOutOfDateKHR: return recreate_swapchain(framebuffer);
	default: break;
	}

	throw Error{"{} failed", op};
}
} // namespace bave
