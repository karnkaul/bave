#pragma once
#include <bave/core/not_null.hpp>
#include <bave/graphics/image_sampler.hpp>
#include <bave/graphics/mesh.hpp>
#include <bave/graphics/render_instance.hpp>
#include <bave/graphics/render_view.hpp>
#include <bave/graphics/set_layout.hpp>
#include <bave/logger.hpp>
#include <map>

namespace bave {
class Shader {
  public:
	struct Program {
		vk::ShaderModule vertex{};
		vk::ShaderModule fragment{};
	};

	explicit Shader(NotNull<class FrameRenderer const*> frame_renderer, Program program, RenderView render_view);

	auto update(std::uint32_t set, std::uint32_t binding, RenderBuffer const& buffer) -> bool;
	auto update(std::uint32_t set, std::uint32_t binding, ImageSampler combined_image_sampler) -> bool;

	void update_textures(std::span<ImageSampler const, SetLayout::max_textures_v> combined_image_samplers);

	void draw(vk::CommandBuffer command_buffer, Mesh const& mesh, std::span<RenderInstance::Baked const> instances);

	float line_width{1.0f};
	vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};

  private:
	auto get_descriptor_set(std::uint32_t set) -> vk::DescriptorSet;

	void set_viewport_scissor();
	void set_view_and_instances(std::span<RenderInstance::Baked const> instances);
	void set_white_textures();

	Logger m_log{"Shader"};

	NotNull<FrameRenderer const*> m_frame_renderer;
	vk::ShaderModule m_vert{};
	vk::ShaderModule m_frag{};
	RenderView m_render_view{};

	std::map<std::uint32_t, vk::DescriptorSet> m_descriptor_sets{};
	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
};
} // namespace bave
