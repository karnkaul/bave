#pragma once
#include <bave/core/not_null.hpp>
#include <bave/graphics/device_blocker.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/render_resource.hpp>

namespace bave {
struct RenderTarget {
	RenderImageView colour{};
	vk::ImageView depth{};
};

class FrameRenderer {
  public:
	explicit FrameRenderer(NotNull<RenderDevice*> render_device);

	[[nodiscard]] auto get_frame_index() const -> FrameIndex { return m_frame.index; }

	auto start_render(vk::ClearColorValue const& clear_colour) -> vk::CommandBuffer;
	auto finish_render() -> bool;

  private:
	struct Frame {
		struct Sync {
			vk::UniqueSemaphore draw{};
			vk::UniqueSemaphore present{};
			vk::UniqueFence drawn{};
			vk::UniqueCommandPool command_pool{};
			vk::CommandBuffer command_buffer{};
		};

		std::unique_ptr<RenderImage> depth_image{};
		Buffered<Sync> syncs{};
		vk::UniqueRenderPass render_pass{};

		Buffered<vk::UniqueFramebuffer> framebuffers{};
		FrameIndex index{};
		std::optional<RenderImageView> swapchain_image{};

		static auto make(RenderDevice& render_device) -> Frame;
	};

	NotNull<RenderDevice*> m_render_device;
	vk::Format m_depth_format{};
	Frame m_frame{};

	DeviceBlocker m_blocker{};
};
} // namespace bave
