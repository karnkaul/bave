#pragma once
#include <bave/graphics/detail/get_bounds.hpp>
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/i_drawable.hpp>
#include <bave/graphics/texture.hpp>
#include <memory>
#include <vector>

namespace bave {
/// \brief Base class for drawable objects.
class Drawable : public IDrawable, public RenderInstance {
  public:
	/// \brief Draw this object using a given shader.
	/// \param shader Shader to use.
	void draw(Shader& shader) const override;

	/// \brief Get the bounding rectangle in world space.
	[[nodiscard]] auto get_bounds() const -> Rect<> { return detail::get_bounds(*this); }
	/// \brief Get the stored Geometry.
	[[nodiscard]] auto get_geometry() const -> Geometry const& { return m_geometry; }
	/// \brief Get the generated RenderPrimitive.
	[[nodiscard]] auto get_render_primitive() const -> RenderPrimitive { return m_primitive; }

	/// \brief Textures to bind during draw.
	std::array<std::shared_ptr<Texture const>, Shader::max_textures_v> textures{};

  protected:
	void set_geometry(Geometry geometry);
	void set_texture(std::shared_ptr<Texture const> texture) { textures.front() = std::move(texture); }

	void update_textures(Shader& out_shader) const;

  private:
	struct Primitive {
		std::vector<std::byte> bytes{};
		std::uint32_t verts{};
		std::uint32_t indices{};
		std::size_t ibo_offset{};
		Topology topology{};

		void write(Geometry const& geometry);
		void clear();

		[[nodiscard]] operator RenderPrimitive() const;
	};

	Geometry m_geometry{};
	Primitive m_primitive{};
};
} // namespace bave
