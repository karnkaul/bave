#pragma once
#include <bave/graphics/gpu.hpp>
#include <bave/graphics/render_target.hpp>
#include <optional>
#include <vector>

namespace bave::detail {
struct Swapchain {
	struct Formats {
		std::vector<vk::SurfaceFormatKHR> srgb{};
		std::vector<vk::SurfaceFormatKHR> linear{};

		static auto make(std::span<vk::SurfaceFormatKHR const> available) -> Formats;
	};

	struct Storage {
		std::vector<RenderTarget> render_targets{};
		std::vector<vk::UniqueImageView> views{};
		vk::UniqueSwapchainKHR swapchain{};
		std::optional<std::uint32_t> image_index{};
	};

	static constexpr auto srgb_formats_v = std::array{vk::Format::eR8G8B8A8Srgb, vk::Format::eB8G8R8A8Srgb, vk::Format::eA8B8G8R8SrgbPack32};

	static constexpr auto is_srgb_format(vk::Format const format) {
		return std::find(srgb_formats_v.begin(), srgb_formats_v.end(), format) != srgb_formats_v.end();
	}

	void make_create_info(vk::SurfaceKHR surface, std::uint32_t queue_family, ColourSpace colour_space);

	Formats formats{};
	std::vector<vk::PresentModeKHR> present_modes{};
	vk::PresentModeKHR desired_present_mode{};
	vk::SwapchainCreateInfoKHR create_info{};
	Storage active{};
};
} // namespace bave::detail
