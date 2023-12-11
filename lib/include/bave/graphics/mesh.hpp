#pragma once
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/render_device.hpp>
#include <memory>

namespace bave {
class Mesh {
  public:
	explicit Mesh(NotNull<RenderDevice*> render_device);

	[[nodiscard]] auto get_vertex_count() const -> std::uint32_t { return m_verts; }
	[[nodiscard]] auto get_index_count() const -> std::uint32_t { return m_indices; }

	void write(Geometry const& geometry);

  private:
	[[nodiscard]] auto get_buffer() const -> Ptr<RenderBuffer>;
	void draw(vk::CommandBuffer command_buffer, std::uint32_t instance_count = 1) const;

	NotNull<RenderDevice const*> m_render_device;
	Defer<Buffered<std::shared_ptr<RenderBuffer>>> m_vbo{};
	std::vector<std::byte> m_data{};
	std::uint32_t m_verts{};
	std::uint32_t m_indices{};
	std::size_t m_ibo_offset{};

	friend class Shader;
};
} // namespace bave
