#pragma once
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/detail/get_bounds.hpp>
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/shader.hpp>
#include <bave/graphics/texture.hpp>
#include <memory>
#include <vector>

namespace bave {
/// \brief Interface for all drawable types.
class IDrawable : public Polymorphic {
  public:
	/// \brief Draw this object using a given shader.
	/// \param shader Shader instance to use.
	virtual void draw(Shader& shader) const = 0;
};

/// \brief Base class for drawable objects.
class Drawable : public IDrawable {
  public:
	/// \brief Draw this object using a given shader.
	/// \param shader Shader instance to use.
	void draw(Shader& shader) const final;

	/// \brief Get the bounding rectangle of this instance.
	[[nodiscard]] auto get_bounds() const -> Rect<> { return detail::get_bounds(*this); }
	/// \brief Get the Geometry of this instance.
	[[nodiscard]] auto get_geometry() const -> Geometry const& { return m_geometry; }

	/// \brief World transform of this instance.
	Transform transform{};
	/// \brief Tint to apply during draw.
	Rgba tint{white_v};
	/// \brief Textures to bind during draw.
	std::array<std::shared_ptr<Texture const>, Shader::max_textures_v> textures{};

	/// \brief Instanced rendering.
	///
	/// transform and tint are ignored if instances is not empty.
	std::vector<RenderInstance> instances{};

  protected:
	void set_geometry(Geometry geometry);
	void set_texture(std::shared_ptr<Texture const> texture) { textures.front() = std::move(texture); }

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

	void bake_instances() const;
	void update_textures(Shader& out_shader) const;

	Geometry m_geometry{};
	Primitive m_primitive{};

	mutable std::vector<RenderInstance::Baked> m_baked_instances{};
};
} // namespace bave
