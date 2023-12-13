#include <bave/core/error.hpp>
#include <bave/graphics/detail/swapchain.hpp>
#include <bave/platform.hpp>
#include <algorithm>

namespace bave::detail {
namespace {
constexpr auto optimal_present_mode(std::span<vk::PresentModeKHR const> modes) -> vk::PresentModeKHR {
	auto ret = vk::PresentModeKHR::eFifoRelaxed;
	if (std::find(modes.begin(), modes.end(), ret) != modes.end()) { return ret; }
	if constexpr (platform_v == Platform::eDesktop) {
		ret = vk::PresentModeKHR::eMailbox;
		if (std::find(modes.begin(), modes.end(), ret) != modes.end()) { return ret; }
	}
	return vk::PresentModeKHR::eFifo;
}
} // namespace

auto Swapchain::Formats::make(std::span<vk::SurfaceFormatKHR const> available) -> Formats {
	auto ret = Formats{};
	for (auto const format : available) {
		if (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			if (is_srgb_format(format.format)) {
				ret.srgb.push_back(format);
			} else {
				ret.linear.push_back(format);
			}
		}
	}
	return ret;
}

void Swapchain::make_create_info(vk::SurfaceKHR surface, std::uint32_t queue_family, ColourSpace colour_space) {
	desired_present_mode = optimal_present_mode(present_modes);
	auto const format = colour_space == ColourSpace::eLinear ? formats.linear.front() : formats.srgb.front();
	create_info.surface = surface;
	create_info.presentMode = desired_present_mode;
	create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	create_info.queueFamilyIndexCount = 1u;
	create_info.pQueueFamilyIndices = &queue_family;
	create_info.imageColorSpace = format.colorSpace;
	create_info.imageArrayLayers = 1u;
	create_info.imageFormat = format.format;
}
} // namespace bave::detail
