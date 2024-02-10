#pragma once
#include <bave/graphics/drawable.hpp>

namespace bave {
/// \brief Base class for Drawable shapes.
class BasicShape : public Drawable {
  public:
	/// \brief Set the primary texture.
	/// \param texture Texture to set. Can be null.
	void set_texture(std::shared_ptr<Texture const> texture) { Drawable::set_texture(std::move(texture)); }
	/// \brief Get the primary texture.
	/// \returns Primary texture. Can be null.
	[[nodiscard]] auto get_texture() const -> std::shared_ptr<Texture const> const& { return textures.front(); }
};

/// \brief Custom shape (geometry managed by user).
class CustomShape : public BasicShape {
  public:
	/// \brief Set the Geometry.
	/// \param geometry Geometry to set.
	void set_geometry(Geometry geometry) { Drawable::set_geometry(std::move(geometry)); }
};

/// \brief Class template for all shapes.
template <typename ShapeT>
class Shape : public BasicShape {
  public:
	Shape() { set_shape(ShapeT{}); }

	void set_shape(ShapeT const& shape) {
		m_shape = shape;
		set_geometry(m_shape.to_geometry());
	}

	void set_origin(glm::vec2 const origin) {
		m_shape.origin = origin;
		set_geometry(m_shape.to_geometry());
	}

	[[nodiscard]] auto get_shape() const -> ShapeT const& { return m_shape; }

  private:
	ShapeT m_shape{};
};

using QuadShape = Shape<Quad>;
using CircleShape = Shape<Circle>;
using RoundedQuadShape = Shape<RoundedQuad>;
using NineQuadShape = Shape<NineQuad>;
using LineRectShape = Shape<LineRect>;
} // namespace bave
