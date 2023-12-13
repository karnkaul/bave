#pragma once
#include <bave/graphics/texture.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace bave::detail {
class SamplerCache {
  public:
	explicit SamplerCache(vk::Device device) : m_device(device) {}

	auto get(Texture::Sampler const& sampler) -> vk::Sampler;

	float anisotropy{};

  private:
	struct Hasher {
		auto operator()(Texture::Sampler const& sampler) const -> std::size_t;
	};

	vk::Device m_device{};
	std::unordered_map<Texture::Sampler, vk::UniqueSampler, Hasher> m_map{};
};
} // namespace bave::detail
