#pragma once
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace bave {
struct Gpu {
	vk::PhysicalDevice device{};
	vk::PhysicalDeviceProperties properties{};
	std::uint32_t queue_family{};
};
} // namespace bave
