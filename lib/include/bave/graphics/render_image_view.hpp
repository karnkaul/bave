#pragma once
#include <vulkan/vulkan.hpp>

namespace bave {
struct RenderImageView {
	vk::Image image{};
	vk::ImageView view{};
	vk::Extent2D extent{};
	vk::Format format{};
};
} // namespace bave
