#pragma once
#include <bave/core/not_null.hpp>
#include <bave/graphics/detail/buffer_type.hpp>
#include <bave/graphics/detail/set_layout.hpp>
#include <bave/graphics/render_instance.hpp>
#include <bave/graphics/render_view.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
class Shader {
  public:
	static constexpr auto max_textures_v = detail::SetLayout::max_textures_v;

	/// \brief The default value for line_width.
	inline static auto default_line_width{1.0f}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
	/// \brief The default value for polygon_mode.
	inline static auto default_polygon_mode{vk::PolygonMode::eFill}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

	explicit Shader(NotNull<class Renderer const*> renderer, vk::ShaderModule vertex, vk::ShaderModule fragment);

	auto update_texture(CombinedImageSampler cis, std::uint32_t binding = 0) -> bool;
	void update_textures(std::span<CombinedImageSampler const, max_textures_v> cis);

	auto write_ubo(void const* data, vk::DeviceSize size) -> bool;
	auto write_ssbo(void const* data, vk::DeviceSize size) -> bool;

	/// \brief Draw intsances of a primitive.
	/// \param primitive Primitive to draw.
	/// \param instances Instances to draw.
	void draw(RenderPrimitive const& primitive, std::span<RenderInstance::Baked const> instances);

	/// \brief Line width (only relevant for vk::PolygonMode::eLine and/or Topology::eLineStrip).
	///
	/// Actual line width will be clamped to render device limits during draw.
	float line_width{default_line_width};
	/// \brief Polygon mode.
	///
	/// Set to vk::PolygonMode::eLine for wireframe mode.
	vk::PolygonMode polygon_mode{default_polygon_mode};

  private:
	struct Sets {
		std::array<CombinedImageSampler, max_textures_v> cis{};
		Ptr<detail::RenderBuffer> ubo{};
		Ptr<detail::RenderBuffer> ssbo{};
	};

	[[nodiscard]] auto allocate_scratch(detail::BufferType type) const -> detail::RenderBuffer&;

	void set_viewport();
	[[nodiscard]] auto get_scissor(Rect<> n_rect) const -> vk::Rect2D;
	void update_and_bind_sets(vk::CommandBuffer command_buffer, std::span<RenderInstance::Baked const> instances) const;

	NotNull<Renderer const*> m_renderer;
	vk::ShaderModule m_vert{};
	vk::ShaderModule m_frag{};

	vk::Viewport m_viewport{};
	Sets m_sets{};
};
} // namespace bave
