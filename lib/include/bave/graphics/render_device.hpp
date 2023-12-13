#pragma once
#include <vk_mem_alloc.h>
#include <bave/core/inclusive_range.hpp>
#include <bave/core/ptr.hpp>
#include <bave/core/scoped_resource.hpp>
#include <bave/font/font_library.hpp>
#include <bave/graphics/buffering.hpp>
#include <bave/graphics/cache/render_buffer_cache.hpp>
#include <bave/graphics/cache/sampler_cache.hpp>
#include <bave/graphics/cache/scratch_buffer_cache.hpp>
#include <bave/graphics/defer.hpp>
#include <bave/graphics/device_blocker.hpp>
#include <bave/graphics/swapchain.hpp>
#include <bave/graphics/wsi.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>
#include <limits>
#include <mutex>

struct GLFWwindow;

namespace bave {
struct RenderDeviceCreateInfo {
	ColourSpace swapchain_colour_space{ColourSpace::eSrgb};
	bool validation_layers{debug_v};
};

class RenderDevice {
  public:
	static constexpr auto max_timeout_v{std::numeric_limits<std::uint64_t>::max()};
	static constexpr auto isr_v = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
	static constexpr auto iv_v = vk::ImageViewType::e2D;

	using CreateInfo = RenderDeviceCreateInfo;

	explicit RenderDevice(NotNull<IWsi*> wsi, CreateInfo create_info = {});

	[[nodiscard]] auto get_instance() const -> vk::Instance { return *m_instance; }
	[[nodiscard]] auto get_surface() const -> vk::SurfaceKHR { return *m_surface; }
	[[nodiscard]] auto get_gpu() const -> Gpu const& { return m_gpu; }
	[[nodiscard]] auto get_device() const -> vk::Device { return *m_device; }
	[[nodiscard]] auto get_queue() const -> vk::Queue { return m_queue; }
	[[nodiscard]] auto get_allocator() const -> VmaAllocator { return m_allocator.get(); }
	[[nodiscard]] auto get_swapchain_format() const -> vk::Format { return m_swapchain.create_info.imageFormat; }
	[[nodiscard]] auto get_font_library() const -> FontLibrary& { return *m_font_library; }

	[[nodiscard]] auto get_defer_queue() -> DeferQueue& { return m_defer_queue; }
	[[nodiscard]] auto get_vertex_buffer_cache() const -> RenderBufferCache& { return *m_vbo_cache; }
	[[nodiscard]] auto get_scratch_buffer_cache() const -> ScratchBufferCache& { return *m_sbo_cache; }
	[[nodiscard]] auto get_sampler_cache() const -> SamplerCache& { return *m_sampler_cache; }

	[[nodiscard]] auto get_line_width_limits() const -> InclusiveRange<float> { return m_line_width_limits; }
	[[nodiscard]] auto get_frame_index() const -> FrameIndex { return m_frame_index; }

	auto wait_for(vk::Fence fence, std::uint64_t timeout = max_timeout_v) const -> bool;
	auto reset_fence(vk::Fence fence, bool wait_first = true) const -> bool;

	auto request_present_mode(vk::PresentModeKHR present_mode) -> bool;

	[[nodiscard]] auto acquire_next_image(vk::Fence wait, vk::Semaphore signal) -> std::optional<RenderTarget>;
	auto queue_submit(vk::SubmitInfo const& submit_info, vk::Fence signal) -> bool;
	auto submit_and_present(vk::SubmitInfo const& submit_info, vk::Fence draw_signal, vk::Semaphore present_wait) -> bool;

	auto recreate_surface() -> bool;

  private:
	struct Deleter {
		void operator()(VmaAllocator allocator) const noexcept { vmaDestroyAllocator(allocator); }
	};

	auto recreate_swapchain(vk::Extent2D framebuffer) -> bool;
	auto handle_swapchain_result(vk::Result result, vk::Extent2D framebuffer, std::string_view op) -> bool;

	Logger m_log{"RenderDevice"};

	mutable std::mutex m_queue_mutex{};

	NotNull<IWsi*> m_wsi;

	vk::UniqueInstance m_instance{};
	vk::UniqueDebugUtilsMessengerEXT m_debug_messenger{};
	vk::UniqueSurfaceKHR m_surface{};
	vk::UniqueDevice m_device{};
	ScopedResource<VmaAllocator, Deleter> m_allocator{};
	DeferQueue m_defer_queue{};
	Gpu m_gpu{};
	vk::Queue m_queue{};
	Swapchain m_swapchain{};
	std::unique_ptr<RenderBufferCache> m_vbo_cache{};
	std::unique_ptr<ScratchBufferCache> m_sbo_cache{};
	std::unique_ptr<SamplerCache> m_sampler_cache{};
	std::unique_ptr<FontLibrary> m_font_library{FontLibrary::make()};

	DeviceBlocker m_blocker{};

	InclusiveRange<float> m_line_width_limits{};
	FrameIndex m_frame_index{};
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
