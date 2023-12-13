#include <bave/core/hash_combine.hpp>
#include <bave/graphics/detail/sampler_cache.hpp>

namespace bave::detail {
namespace {
constexpr auto from(Texture::Wrap const wrap) -> vk::SamplerAddressMode {
	switch (wrap) {
	case Texture::Wrap::eClampBorder: return vk::SamplerAddressMode::eClampToBorder;
	case Texture::Wrap::eClampEdge: return vk::SamplerAddressMode::eClampToEdge;
	default: return vk::SamplerAddressMode::eRepeat;
	}
}

constexpr auto from(Texture::Filter const filter) -> vk::Filter {
	switch (filter) {
	case Texture::Filter::eNearest: return vk::Filter::eNearest;
	default: return vk::Filter::eLinear;
	}
}

constexpr auto from(Texture::Border const border) -> vk::BorderColor {
	switch (border) {
	default:
	case Texture::Border::eOpaqueBlack: return vk::BorderColor::eFloatOpaqueBlack;
	case Texture::Border::eOpaqueWhite: return vk::BorderColor::eFloatOpaqueWhite;
	case Texture::Border::eTransparentBlack: return vk::BorderColor::eFloatTransparentBlack;
	}
}
} // namespace

auto SamplerCache::Hasher::operator()(Texture::Sampler const& sampler) const -> std::size_t {
	return make_combined_hash(sampler.min, sampler.mag, sampler.wrap_s, sampler.wrap_t, sampler.border);
}

auto SamplerCache::get(Texture::Sampler const& sampler) -> vk::Sampler {
	if (auto it = m_map.find(sampler); it != m_map.end()) { return *it->second; }
	auto sci = vk::SamplerCreateInfo{};
	sci.minFilter = from(sampler.min);
	sci.magFilter = from(sampler.mag);
	sci.anisotropyEnable = anisotropy > 0.0f ? 1 : 0;
	sci.maxAnisotropy = anisotropy;
	sci.borderColor = from(sampler.border);
	sci.mipmapMode = vk::SamplerMipmapMode::eNearest;
	sci.addressModeU = from(sampler.wrap_s);
	sci.addressModeV = from(sampler.wrap_t);
	sci.addressModeW = from(sampler.wrap_s);
	sci.maxLod = VK_LOD_CLAMP_NONE;
	auto [it, _] = m_map.insert_or_assign(sampler, m_device.createSamplerUnique(sci));
	return *it->second;
}
} // namespace bave::detail
