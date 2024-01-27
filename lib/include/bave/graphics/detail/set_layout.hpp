#pragma once
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace bave::detail {
template <std::size_t Count>
constexpr auto make_bindings(vk::DescriptorType const type) -> std::array<vk::DescriptorType, Count> {
	auto ret = std::array<vk::DescriptorType, Count>{};
	for (auto& out_type : ret) { out_type = type; }
	return ret;
}

struct SetLayout {
	template <std::size_t Bindings>
	struct Set {
		std::uint32_t set{};
		std::array<vk::DescriptorType, Bindings> bindings{};
	};

	static constexpr std::uint32_t max_textures_v{4};

	Set<2> view_instances = Set<2>{
		.set = 0,
		.bindings = {vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eStorageBuffer},
	};

	Set<max_textures_v> textures = Set<max_textures_v>{
		.set = 1,
		.bindings = make_bindings<max_textures_v>(vk::DescriptorType::eCombinedImageSampler),
	};

	Set<2> buffers = Set<2>{
		.set = 2,
		.bindings = {vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eStorageBuffer},
	};
};

constexpr auto set_layout_v = SetLayout{};
} // namespace bave::detail
