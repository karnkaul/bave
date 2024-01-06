#pragma once
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/render_device.hpp>
#include <memory>

namespace bave {
class Mesh {
  public:
	Mesh(Mesh const&) = delete;
	auto operator=(Mesh const&) -> Mesh& = delete;

	Mesh(Mesh&&) = default;
	auto operator=(Mesh&&) -> Mesh& = default;

	explicit Mesh(NotNull<RenderDevice*> render_device);

	~Mesh();

	[[nodiscard]] auto get_render_device() const -> RenderDevice const& { return *m_render_device; }

	[[nodiscard]] auto get_vertex_count() const -> std::uint32_t { return m_verts; }
	[[nodiscard]] auto get_index_count() const -> std::uint32_t { return m_indices; }

	void write(Geometry const& geometry);

  private:
	[[nodiscard]] auto get_buffer() const -> Ptr<detail::RenderBuffer>;
	void draw(vk::CommandBuffer command_buffer, std::uint32_t instance_count = 1) const;

	NotNull<RenderDevice*> m_render_device;
	std::shared_ptr<detail::VertexBuffer> m_vbo{};
	std::vector<std::byte> m_data{};
	std::uint32_t m_verts{};
	std::uint32_t m_indices{};
	std::size_t m_ibo_offset{};

	friend class Shader;
};
} // namespace bave
