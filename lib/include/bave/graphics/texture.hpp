#pragma once
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/graphics/sampler.hpp>

namespace bave {
struct CombinedImageSampler {
	vk::ImageView image_view{};
	vk::Sampler sampler{};
};

class Texture {
  public:
	explicit Texture(NotNull<RenderDevice*> render_device, bool mip_map = false);
	explicit Texture(NotNull<RenderDevice*> render_device, BitmapView bitmap, bool mip_map = false);

	auto load_image(std::span<std::byte const> compressed, bool mip_map = false) -> bool;
	void write(BitmapView bitmap, bool mip_map = false);

	[[nodiscard]] auto combined_image_sampler() const -> CombinedImageSampler;

	[[nodiscard]] auto get_image() const -> detail::RenderImage const& { return m_image; }

	Sampler sampler{};

  private:
	detail::RenderImage m_image;
};
} // namespace bave
