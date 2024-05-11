#pragma once
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/shader.hpp>

namespace bave {
/// \brief Interface for all drawable types.
class IDrawable : public Polymorphic {
  public:
	/// \brief Draw this object using a given shader.
	/// \param shader Shader to use.
	virtual void draw(Shader& shader) const = 0;
};
} // namespace bave
