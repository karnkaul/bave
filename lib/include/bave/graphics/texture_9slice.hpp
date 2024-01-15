#pragma once
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
class Texture9Slice : public Texture {
  public:
	explicit Texture9Slice(NotNull<RenderDevice*> render_device, BitmapView bitmap, NineSlice slice) : Texture(render_device, bitmap, false), m_slice(slice) {}

	[[nodiscard]] auto get_slice() const -> NineSlice const& { return m_slice; }

  private:
	NineSlice m_slice;
};
} // namespace bave
