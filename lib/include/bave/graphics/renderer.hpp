#pragma once
#include <bave/core/not_null.hpp>
#include <bave/core/pinned.hpp>
#include <bave/graphics/cache/pipeline_cache.hpp>
#include <bave/graphics/device_blocker.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/render_resource.hpp>
#include <bave/graphics/render_view.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/texture.hpp>
#include <bave/transform.hpp>
#include <optional>

namespace bave {
class Renderer : public Pinned {
  public:
	explicit Renderer(NotNull<RenderDevice*> render_device, NotNull<DataStore const*> data_store, NotNull<RenderView const*> render_view);

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return *m_render_device; }
	[[nodiscard]] auto get_frame_index() const -> FrameIndex { return m_render_device->get_frame_index(); }
	[[nodiscard]] auto get_render_pass() const -> vk::RenderPass { return *m_frame.render_pass; }

	[[nodiscard]] auto get_pipeline_cache() const -> PipelineCache& { return *m_pipeline_cache; }

	auto start_render(Rgba clear_colour) -> vk::CommandBuffer;
	auto finish_render() -> bool;

	[[nodiscard]] auto is_rendering() const -> bool { return m_frame.render_target.has_value(); }
	[[nodiscard]] auto get_white_texture() const -> Texture const& { return m_white; }
	[[nodiscard]] auto or_white(Ptr<Texture const> texture) const -> Texture const& { return texture != nullptr ? *texture : m_white; }

	[[nodiscard]] auto get_backbuffer_extent() const -> vk::Extent2D;

	NotNull<RenderView const*> render_view;

  private:
	struct Frame {
		struct Sync {
			vk::UniqueSemaphore draw{};
			vk::UniqueSemaphore present{};
			vk::UniqueFence drawn{};
			vk::UniqueCommandPool command_pool{};
			vk::CommandBuffer command_buffer{};
		};

		Buffered<Sync> syncs{};
		vk::UniqueRenderPass render_pass{};

		Buffered<vk::UniqueFramebuffer> framebuffers{};
		std::optional<RenderTarget> render_target{};

		static auto make(RenderDevice& render_device) -> Frame;
	};

	Logger m_log{"FrameRenderer"};

	NotNull<RenderDevice*> m_render_device;
	Frame m_frame{};
	std::unique_ptr<PipelineCache> m_pipeline_cache{};
	Texture m_white;

	DeviceBlocker m_blocker{};
};
} // namespace bave
