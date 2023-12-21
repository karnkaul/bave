#pragma once
#include <bave/core/not_null.hpp>
#include <bave/graphics/mesh.hpp>
#include <bave/graphics/render_instance.hpp>
#include <bave/graphics/render_view.hpp>
#include <bave/graphics/set_layout.hpp>
#include <bave/graphics/texture.hpp>
#include <bave/logger.hpp>

namespace bave {
class Shader {
  public:
	explicit Shader(NotNull<class Renderer const*> renderer, vk::ShaderModule vertex, vk::ShaderModule fragment);

	auto update_texture(CombinedImageSampler cis, std::uint32_t binding = 0) -> bool;
	void update_textures(std::span<CombinedImageSampler const, SetLayout::max_textures_v> cis);

	auto write_ubo(void const* data, vk::DeviceSize size) -> bool;
	auto write_ssbo(void const* data, vk::DeviceSize size) -> bool;

	void draw(vk::CommandBuffer command_buffer, Mesh const& mesh, std::span<RenderInstance::Baked const> instances);

	float line_width{1.0f};
	vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};

  private:
	struct Sets {
		std::array<CombinedImageSampler, SetLayout::max_textures_v> cis{};
		Ptr<detail::RenderBuffer> ubo{};
		Ptr<detail::RenderBuffer> ssbo{};
	};

	[[nodiscard]] auto allocate_scratch(vk::BufferUsageFlagBits usage) const -> detail::RenderBuffer&;

	auto update(vk::DescriptorSet descriptor_set, std::uint32_t binding, detail::RenderBuffer const& buffer) -> bool;

	template <vk::BufferUsageFlagBits Usage>
	auto write(vk::DescriptorSet set, std::uint32_t binding, void const* data, vk::DeviceSize size) -> bool {
		auto& scratch_buffer = allocate_scratch(Usage);
		scratch_buffer.write(data, size);
		return update(set, binding, scratch_buffer);
	}

	void set_viewport_scissor();
	void write_view_and_instances(vk::DescriptorSet descriptor_set, std::span<RenderInstance::Baked const> instances);
	void update_textures(vk::DescriptorSet descriptor_set);
	void update_buffers(vk::DescriptorSet descriptor_set);

	Logger m_log{"Shader"};

	NotNull<Renderer const*> m_renderer;
	vk::ShaderModule m_vert{};
	vk::ShaderModule m_frag{};

	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
	Sets m_sets{};
};
} // namespace bave
