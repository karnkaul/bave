#pragma once
#include <vk_mem_alloc.h>
#include <bave/core/inclusive_range.hpp>
#include <bave/core/ptr.hpp>
#include <bave/core/scoped_resource.hpp>
#include <bave/font/detail/font_library.hpp>
#include <bave/graphics/detail/buffer_cache.hpp>
#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/defer.hpp>
#include <bave/graphics/detail/device_blocker.hpp>
#include <bave/graphics/detail/image_cache.hpp>
#include <bave/graphics/detail/sampler_cache.hpp>
#include <bave/graphics/detail/swapchain.hpp>
#include <bave/graphics/detail/wsi.hpp>
#include <bave/graphics/extent_scaler.hpp>
#include <bave/graphics/render_view.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>
#include <limits>
#include <mutex>

struct GLFWwindow;

namespace bave {
struct RenderDeviceCreateInfo {
	detail::ColourSpace swapchain_colour_space{detail::ColourSpace::eSrgb};
	vk::SampleCountFlagBits desired_samples{vk::SampleCountFlagBits::e1};
	bool validation_layers{debug_v};
};

/// \brief Vulkan rendering device.
class RenderDevice {
  public:
	static constexpr auto max_timeout_v{std::numeric_limits<std::uint64_t>::max()};
	static constexpr auto isr_v = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
	static constexpr auto iv_v = vk::ImageViewType::e2D;

	using CreateInfo = RenderDeviceCreateInfo;

	explicit RenderDevice(NotNull<detail::IWsi*> wsi, CreateInfo create_info = {});

	[[nodiscard]] auto validation_layers_enabled() const -> bool { return !!m_debug_messenger; }

	[[nodiscard]] auto get_instance() const -> vk::Instance { return *m_instance; }
	[[nodiscard]] auto get_surface() const -> vk::SurfaceKHR { return *m_surface; }
	[[nodiscard]] auto get_gpu() const -> Gpu const& { return m_gpu; }
	[[nodiscard]] auto get_device() const -> vk::Device { return *m_device; }
	[[nodiscard]] auto get_queue() const -> vk::Queue { return m_queue; }
	[[nodiscard]] auto get_allocator() const -> VmaAllocator { return m_allocator.get(); }
	[[nodiscard]] auto get_swapchain_format() const -> vk::Format { return m_swapchain.create_info.imageFormat; }
	[[nodiscard]] auto get_swapchain_extent() const -> vk::Extent2D { return m_swapchain.create_info.imageExtent; }

	[[nodiscard]] auto get_line_width_limits() const -> InclusiveRange<float> { return m_line_width_limits; }
	[[nodiscard]] auto get_sample_count() const -> vk::SampleCountFlagBits { return m_samples; }
	[[nodiscard]] auto get_frame_index() const -> detail::FrameIndex { return m_frame_index; }
	[[nodiscard]] auto get_default_view() const -> RenderView;

	[[nodiscard]] auto get_viewport_scaler() const -> ExtentScaler;
	[[nodiscard]] auto project_to(glm::vec2 target_space, glm::vec2 point) const -> glm::vec2;

	auto wait_for(vk::Fence fence, std::uint64_t timeout = max_timeout_v) const -> bool;
	auto reset_fence(vk::Fence fence, bool wait_first = true) const -> bool;

	auto request_present_mode(vk::PresentModeKHR present_mode) -> bool;

	[[nodiscard]] auto acquire_next_image(vk::Fence wait, vk::Semaphore signal) -> std::optional<detail::RenderTarget>;
	auto queue_submit(vk::SubmitInfo const& submit_info, vk::Fence signal) -> bool;
	auto submit_and_present(vk::SubmitInfo const& submit_info, vk::Fence draw_signal, vk::Semaphore present_wait) -> bool;

	auto recreate_surface() -> bool;

	[[nodiscard]] auto get_defer_queue() -> detail::DeferQueue& { return m_defer_queue; }
	[[nodiscard]] auto get_buffer_cache() const -> detail::BufferCache& { return *m_buffer_cache; }
	[[nodiscard]] auto get_image_cache() const -> detail::ImageCache& { return *m_image_cache; }
	[[nodiscard]] auto get_sampler_cache() const -> detail::SamplerCache& { return *m_sampler_cache; }
	[[nodiscard]] auto get_font_library() const -> detail::FontLibrary& { return *m_font_library; }

	RenderView render_view{};

  private:
	struct Deleter {
		void operator()(VmaAllocator allocator) const noexcept { vmaDestroyAllocator(allocator); }
	};

	auto recreate_swapchain(vk::Extent2D framebuffer) -> bool;
	auto handle_swapchain_result(vk::Result result, vk::Extent2D framebuffer, std::string_view op) -> bool;

	Logger m_log{"RenderDevice"};

	mutable std::mutex m_queue_mutex{};

	NotNull<detail::IWsi*> m_wsi;

	vk::UniqueInstance m_instance{};
	vk::UniqueDebugUtilsMessengerEXT m_debug_messenger{};
	vk::UniqueSurfaceKHR m_surface{};
	vk::UniqueDevice m_device{};
	ScopedResource<VmaAllocator, Deleter> m_allocator{};
	detail::DeferQueue m_defer_queue{};
	Gpu m_gpu{};
	vk::Queue m_queue{};
	detail::Swapchain m_swapchain{};
	std::unique_ptr<detail::BufferCache> m_buffer_cache{};
	std::unique_ptr<detail::ImageCache> m_image_cache{};
	std::unique_ptr<detail::SamplerCache> m_sampler_cache{};
	std::unique_ptr<detail::FontLibrary> m_font_library{detail::FontLibrary::make()};

	detail::DeviceBlocker m_blocker{};

	InclusiveRange<float> m_line_width_limits{};
	vk::SampleCountFlagBits m_samples{};
	detail::FrameIndex m_frame_index{};
};

constexpr auto to_vsync_string(vk::PresentModeKHR const mode) -> std::string_view {
	switch (mode) {
	case vk::PresentModeKHR::eFifo: return "classic";
	case vk::PresentModeKHR::eFifoRelaxed: return "adaptive";
	case vk::PresentModeKHR::eMailbox: return "mailbox";
	case vk::PresentModeKHR::eImmediate: return "immediate";
	default: return "unsupported";
	}
}
} // namespace bave
