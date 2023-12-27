#pragma once
#include <bave/core/not_null.hpp>
#include <bave/graphics/mesh.hpp>
#include <bave/graphics/render_instance.hpp>
#include <bave/graphics/render_view.hpp>
#include <bave/graphics/set_layout.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
class Shader {
  public:
	explicit Shader(NotNull<class Renderer const*> renderer, vk::ShaderModule vertex, vk::ShaderModule fragment);

	auto update_texture(CombinedImageSampler cis, std::uint32_t binding = 0) -> bool;
	void update_textures(std::span<CombinedImageSampler const, SetLayout::max_textures_v> cis);

	auto write_ubo(void const* data, vk::DeviceSize size) -> bool;
	auto write_ssbo(void const* data, vk::DeviceSize size) -> bool;

	void draw(Mesh const& mesh, std::span<RenderInstance::Baked const> instances);

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

	void set_viewport_scissor();
	void update_and_bind_sets(vk::CommandBuffer command_buffer, std::span<RenderInstance::Baked const> instances) const;

	NotNull<Renderer const*> m_renderer;
	vk::ShaderModule m_vert{};
	vk::ShaderModule m_frag{};

	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
	Sets m_sets{};
};
} // namespace bave
