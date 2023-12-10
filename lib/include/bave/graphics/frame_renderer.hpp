#pragma once
#include <bave/core/not_null.hpp>
#include <bave/core/pinned.hpp>
#include <bave/graphics/cache/pipeline_cache.hpp>
#include <bave/graphics/device_blocker.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/render_resource.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/shader.hpp>
#include <optional>

namespace bave {
struct RenderTarget {
	RenderImageView colour{};
	vk::ImageView depth{};
};

class FrameRenderer : public Pinned {
  public:
	explicit FrameRenderer(NotNull<RenderDevice*> render_device, NotNull<DataStore const*> data_store);

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return *m_render_device; }
	[[nodiscard]] auto get_frame_index() const -> FrameIndex { return m_render_device->get_frame_index(); }
	[[nodiscard]] auto get_render_pass() const -> vk::RenderPass { return *m_frame.render_pass; }
	[[nodiscard]] auto get_pipeline_cache() const -> PipelineCache& { return *m_pipeline_cache; }

	auto start_render(Rgba clear_colour) -> vk::CommandBuffer;
	auto finish_render() -> bool;

	[[nodiscard]] auto is_rendering() const -> bool { return m_frame.swapchain_image.has_value(); }

	// in render pass
	[[nodiscard]] auto load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader>;
	[[nodiscard]] auto get_backbuffer_extent() const -> vk::Extent2D;

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
		std::optional<RenderImageView> swapchain_image{};

		static auto make(RenderDevice& render_device) -> Frame;
	};

	Logger m_log{"FrameRenderer"};

	NotNull<RenderDevice*> m_render_device;
	std::unique_ptr<PipelineCache> m_pipeline_cache{};
	vk::Format m_depth_format{};
	Frame m_frame{};

	DeviceBlocker m_blocker{};
};
} // namespace bave
