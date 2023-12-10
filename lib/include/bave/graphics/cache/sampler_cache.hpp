#pragma once
#include <bave/graphics/sampler.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace bave {
class SamplerCache {
  public:
	explicit SamplerCache(vk::Device device) : m_device(device) {}

	auto get(Sampler const& sampler) -> vk::Sampler;

	float anisotropy{};

  private:
	struct Hasher {
		auto operator()(Sampler const& sampler) const -> std::size_t;
	};

	vk::Device m_device{};
	std::unordered_map<Sampler, vk::UniqueSampler, Hasher> m_map{};
};
} // namespace bave
