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
	explicit Shader(NotNull<class Renderer const*> renderer, vk::ShaderModule vertex, vk::ShaderModule fragment);

	auto update_texture(ImageSampler combined_image_sampler, std::uint32_t binding = 0) -> bool;
	void update_textures(std::span<ImageSampler const, SetLayout::max_textures_v> combined_image_samplers);

	auto write_ubo(void const* data, vk::DeviceSize size) -> bool;
	auto write_ssbo(void const* data, vk::DeviceSize size) -> bool;

	void draw(vk::CommandBuffer command_buffer, Mesh const& mesh, std::span<RenderInstance::Baked const> instances);

	float line_width{1.0f};
	vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};

  private:
	auto get_descriptor_set(std::uint32_t set) -> vk::DescriptorSet;

	[[nodiscard]] auto allocate_scratch(vk::BufferUsageFlagBits usage) const -> RenderBuffer&;

	auto update(vk::DescriptorSet descriptor_set, std::uint32_t binding, RenderBuffer const& buffer) -> bool;

	template <vk::BufferUsageFlagBits Usage>
	auto write(vk::DescriptorSet set, std::uint32_t binding, void const* data, vk::DeviceSize size) -> bool {
		auto& scratch_buffer = allocate_scratch(Usage);
		scratch_buffer.write(data, size);
		return update(set, binding, scratch_buffer);
	}

	void set_viewport_scissor();
	void write_view_and_instances(std::span<RenderInstance::Baked const> instances);
	void set_white_textures();
	void set_blank_buffers();

	Logger m_log{"Shader"};

	NotNull<Renderer const*> m_renderer;
	vk::ShaderModule m_vert{};
	vk::ShaderModule m_frag{};

	std::map<std::uint32_t, vk::DescriptorSet> m_descriptor_sets{};
	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
};
} // namespace bave
