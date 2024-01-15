#pragma once
#include <bave/graphics/detail/render_resource.hpp>
#include <memory>

namespace bave {
struct CombinedImageSampler {
	vk::ImageView image_view{};
	vk::Sampler sampler{};
};

class Texture {
  public:
	enum class Wrap : int { eRepeat, eClampEdge, eClampBorder };
	enum class Filter : int { eLinear, eNearest };
	enum class Border : int { eOpaqueBlack, eOpaqueWhite, eTransparentBlack };

	struct Sampler {
		Wrap wrap_s{Wrap::eRepeat};
		Wrap wrap_t{Wrap::eRepeat};
		Filter min{Filter::eLinear};
		Filter mag{Filter::eLinear};
		Border border{Border::eOpaqueBlack};

		auto operator==(Sampler const&) const -> bool = default;
	};

	Texture(Texture const&) = delete;
	auto operator=(Texture const&) -> Texture& = delete;

	Texture(Texture&&) = default;
	auto operator=(Texture&&) -> Texture& = default;

	explicit Texture(NotNull<RenderDevice*> render_device, BitmapView bitmap, bool mip_map = false);

	~Texture();

	[[nodiscard]] auto get_size() const -> glm::ivec2;
	[[nodiscard]] auto combined_image_sampler() const -> CombinedImageSampler;

	[[nodiscard]] auto get_image() const -> std::shared_ptr<detail::RenderImage> const& { return m_image; }

	Sampler sampler{};

  protected:
	NotNull<RenderDevice*> m_render_device;
	std::shared_ptr<detail::RenderImage> m_image{};
};

class TextureWriteable : public Texture {
  public:
	explicit TextureWriteable(NotNull<RenderDevice*> render_device, BitmapView bitmap = {}, bool mip_map = false) : Texture(render_device, bitmap, mip_map) {}

	auto load_from_bytes(std::span<std::byte const> compressed) -> bool;
	void write(BitmapView bitmap);
};
} // namespace bave
