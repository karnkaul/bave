#pragma once
#include <vulkan/vulkan.hpp>

namespace bave::detail {
enum class ColourSpace : int { eSrgb, eLinear };

struct RenderTarget {
	vk::Image image{};
	vk::ImageView view{};
	vk::Extent2D extent{};
	vk::Format format{};
};
} // namespace bave::detail
