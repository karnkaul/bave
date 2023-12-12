#pragma once
#include <vulkan/vulkan.hpp>

namespace bave {
struct SetLayout {
	template <std::size_t Bindings>
	struct Set {
		std::uint32_t set{};
		std::array<vk::DescriptorType, Bindings> bindings{};
	};

	static constexpr std::uint32_t max_textures_v{4};

	static constexpr auto make_textures_set(std::uint32_t const set) -> Set<max_textures_v> {
		auto ret = Set<max_textures_v>{.set = set};
		for (auto& type : ret.bindings) { type = vk::DescriptorType::eCombinedImageSampler; }
		return ret;
	}

	Set<2> view_instances = Set<2>{
		.set = 0,
		.bindings = {vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eStorageBuffer},
	};

	Set<max_textures_v> textures = make_textures_set(1);

	Set<2> buffers = Set<2>{
		.set = 2,
		.bindings = {vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eStorageBuffer},
	};
};

constexpr auto set_layout_v = SetLayout{};
} // namespace bave
