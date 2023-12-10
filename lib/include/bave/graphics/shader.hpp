#pragma once
#include <bave/core/not_null.hpp>
#include <bave/graphics/pipeline_state.hpp>
#include <bave/graphics/render_resource.hpp>
#include <bave/logger.hpp>
#include <map>

namespace bave {
class Shader {
  public:
	explicit Shader(NotNull<class FrameRenderer const*> frame_renderer, vk::ShaderModule vert, vk::ShaderModule frag);

	auto update(std::uint32_t set, std::uint32_t binding, RenderBuffer const& buffer) -> bool;
	auto update(std::uint32_t set, std::uint32_t binding, RenderImage const& image, vk::Sampler sampler) -> bool;

	void draw(vk::CommandBuffer command_buffer, std::uint32_t vertex_count, std::uint32_t instance_count = 1);

	PipelineState pipeline_state{};
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};

  private:
	auto get_descriptor_set(std::uint32_t set) -> vk::DescriptorSet;

	Logger m_log{"Shader"};
	NotNull<FrameRenderer const*> m_frame_renderer;
	std::map<std::uint32_t, vk::DescriptorSet> m_descriptor_sets{};
	vk::ShaderModule m_vert{};
	vk::ShaderModule m_frag{};
	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
};
} // namespace bave
