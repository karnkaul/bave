#pragma once
#include <bave/core/not_null.hpp>
#include <bave/graphics/image_sampler.hpp>
#include <bave/graphics/mesh.hpp>
#include <bave/graphics/render_instance.hpp>
#include <bave/logger.hpp>
#include <map>

namespace bave {
class Shader {
  public:
	explicit Shader(NotNull<class FrameRenderer const*> frame_renderer, vk::ShaderModule vert, vk::ShaderModule frag);

	auto update(std::uint32_t set, std::uint32_t binding, RenderBuffer const& buffer) -> bool;
	auto update(std::uint32_t set, std::uint32_t binding, ImageSampler combined_image_sampler, std::uint32_t index = 0) -> bool;

	void draw(vk::CommandBuffer command_buffer, Mesh const& mesh, std::span<RenderInstance::Baked const> instances);

	float line_width{1.0f};
	vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};

  private:
	auto get_descriptor_set(std::uint32_t set) -> vk::DescriptorSet;

	void write_view_set();
	void write_instances_set(std::span<RenderInstance::Baked const> instances);

	Logger m_log{"Shader"};
	NotNull<FrameRenderer const*> m_frame_renderer;
	std::map<std::uint32_t, vk::DescriptorSet> m_descriptor_sets{};
	vk::ShaderModule m_vert{};
	vk::ShaderModule m_frag{};
	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
};
} // namespace bave
