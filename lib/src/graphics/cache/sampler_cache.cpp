#include <bave/core/hash_combine.hpp>
#include <bave/graphics/cache/sampler_cache.hpp>

namespace bave {
namespace {
constexpr auto from(Sampler::Wrap const wrap) -> vk::SamplerAddressMode {
	switch (wrap) {
	case Sampler::Wrap::eClampBorder: return vk::SamplerAddressMode::eClampToBorder;
	case Sampler::Wrap::eClampEdge: return vk::SamplerAddressMode::eClampToEdge;
	default: return vk::SamplerAddressMode::eRepeat;
	}
}

constexpr auto from(Sampler::Filter const filter) -> vk::Filter {
	switch (filter) {
	case Sampler::Filter::eNearest: return vk::Filter::eNearest;
	default: return vk::Filter::eLinear;
	}
}

constexpr auto from(Sampler::Border const border) -> vk::BorderColor {
	switch (border) {
	default:
	case Sampler::Border::eOpaqueBlack: return vk::BorderColor::eFloatOpaqueBlack;
	case Sampler::Border::eOpaqueWhite: return vk::BorderColor::eFloatOpaqueWhite;
	case Sampler::Border::eTransparentBlack: return vk::BorderColor::eFloatTransparentBlack;
	}
}
} // namespace

auto SamplerCache::Hasher::operator()(Sampler const& sampler) const -> std::size_t {
	return make_combined_hash(sampler.min, sampler.mag, sampler.wrap_s, sampler.wrap_t, sampler.border);
}

auto SamplerCache::get(Sampler const& sampler) -> vk::Sampler {
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
} // namespace bave
