#pragma once
#include <bave/drawable.hpp>

namespace bave {
class BasicShape : public Drawable {
  public:
	using Drawable::Drawable;

	void set_texture(std::shared_ptr<Texture const> texture) { textures.front() = std::move(texture); }
};

class CustomShape : public BasicShape {
  public:
	using BasicShape::BasicShape;

	void set_geometry(Geometry const& geometry) { Drawable::set_geometry(geometry); }
};

template <typename ShapeT>
class Shape : public BasicShape {
  public:
	explicit Shape(NotNull<RenderDevice*> render_device, ShapeT const& shape = ShapeT{}) : BasicShape(render_device) { set_shape(shape); }

	void set_shape(ShapeT const& shape) {
		m_shape = shape;
		set_geometry(shape.to_geometry());
	}

  private:
	ShapeT m_shape{};
};

using QuadShape = Shape<Quad>;
using CircleShape = Shape<Circle>;
using RoundedQuadShape = Shape<RoundedQuad>;
using NineSliceShape = Shape<NineSlice>;
} // namespace bave
