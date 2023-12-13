#pragma once
#include <bave/graphics/image_sampler.hpp>
#include <bave/graphics/render_resource.hpp>
#include <bave/graphics/sampler.hpp>

namespace bave {
class Texture {
  public:
	explicit Texture(NotNull<RenderDevice*> render_device, bool mip_map = false);
	explicit Texture(NotNull<RenderDevice*> render_device, BitmapView bitmap, bool mip_map = false);

	auto load_image(std::span<std::byte const> compressed, bool mip_map = false) -> bool;
	void write(BitmapView bitmap, bool mip_map = false);

	[[nodiscard]] auto get_image() const -> RenderImage const& { return m_image; }

	[[nodiscard]] auto combined_image_sampler() const -> ImageSampler;

	Sampler sampler{};

  private:
	RenderImage m_image;
};
} // namespace bave
