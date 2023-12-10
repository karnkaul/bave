#pragma once
#include <vulkan/vulkan.hpp>

namespace bave {
struct ImageSampler {
	vk::ImageView image_view{};
	vk::Sampler sampler{};
};
} // namespace bave
