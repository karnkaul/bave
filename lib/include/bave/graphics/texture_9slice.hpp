#pragma once
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
/// \brief 9-sliced texture.
class Texture9Slice : public Texture {
  public:
	/// \brief Constructor.
	/// \param render_device Non-null pointer to RenderDevice.
	/// \param bitmap View of 9-slice bitmap.
	/// \param slice NineSlice instance.
	explicit Texture9Slice(NotNull<RenderDevice*> render_device, BitmapView bitmap, NineSlice slice) : Texture(render_device, bitmap, false), m_slice(slice) {}

	[[nodiscard]] auto get_slice() const -> NineSlice const& { return m_slice; }

  private:
	NineSlice m_slice;
};
} // namespace bave
