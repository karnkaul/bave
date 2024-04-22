#pragma once
#include <bave/core/not_null.hpp>
#include <bave/core/pinned.hpp>
#include <bave/detail/data_store_provider.hpp>
#include <bave/graphics/detail/device_blocker.hpp>
#include <bave/graphics/detail/pipeline_cache.hpp>
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/graphics/texture.hpp>
#include <bave/graphics/transform.hpp>
#include <optional>

namespace bave {
class Renderer : public Pinned {
  public:
	explicit Renderer(NotNull<RenderDevice*> render_device, NotNull<detail::DataStoreProvider const*> data_store_provider);

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return *m_render_device; }

	auto start_render(Rgba clear_colour) -> bool;
	auto finish_render() -> bool;

	[[nodiscard]] auto is_rendering() const -> bool { return m_frame.render_target.has_value(); }
	[[nodiscard]] auto get_white_texture() const -> Texture const& { return m_white; }
	[[nodiscard]] auto or_white(Ptr<Texture const> texture) const -> Texture const& { return texture != nullptr ? *texture : m_white; }

	[[nodiscard]] auto get_backbuffer_extent() const -> vk::Extent2D;

	[[nodiscard]] auto get_render_pass() const -> vk::RenderPass { return *m_frame.render_pass; }
	[[nodiscard]] auto get_frame_index() const -> detail::FrameIndex { return m_render_device->get_frame_index(); }
	[[nodiscard]] auto get_pipeline_cache() const -> detail::PipelineCache& { return *m_pipeline_cache; }

	[[nodiscard]] auto get_command_buffer() const -> vk::CommandBuffer;

  private:
	struct Frame {
		struct Sync {
			vk::UniqueSemaphore draw{};
			vk::UniqueSemaphore present{};
			vk::UniqueFence drawn{};
			vk::UniqueCommandPool command_pool{};
			vk::CommandBuffer command_buffer{};
		};

		detail::Buffered<Sync> syncs{};
		std::optional<detail::RenderImage> msaa_image{};
		vk::UniqueRenderPass render_pass{};

		detail::Buffered<vk::UniqueFramebuffer> framebuffers{};
		std::optional<detail::RenderTarget> render_target{};

		static auto make(RenderDevice& render_device) -> Frame;
		void make_syncs(vk::Device device, std::uint32_t queue_family);
	};

	Logger m_log{"FrameRenderer"};

	NotNull<RenderDevice*> m_render_device;
	Frame m_frame{};
	std::unique_ptr<detail::PipelineCache> m_pipeline_cache{};
	Texture m_white;

	detail::DeviceBlocker m_blocker{};
};
} // namespace bave
