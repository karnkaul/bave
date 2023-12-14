#pragma once
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace bave::detail {
class RenderImage;

struct ImageBarrier {
	vk::ImageMemoryBarrier barrier{};
	vk::PipelineStageFlags src_stage{};
	vk::PipelineStageFlags dst_stage{};

	explicit ImageBarrier(vk::Image image, std::uint32_t mip_levels = 1, std::uint32_t array_layers = 1);

	explicit ImageBarrier(RenderImage const& image);

	auto set_full_barrier(vk::ImageLayout src, vk::ImageLayout dst) -> ImageBarrier&;
	auto set_undef_to_optimal(bool depth) -> ImageBarrier&;
	auto set_undef_to_transfer_dst() -> ImageBarrier&;
	auto set_optimal_to_read_only(bool depth) -> ImageBarrier&;
	auto set_optimal_to_transfer_src() -> ImageBarrier&;
	auto set_optimal_to_present() -> ImageBarrier&;
	auto set_transfer_dst_to_optimal(bool depth) -> ImageBarrier&;
	auto set_transfer_dst_to_present() -> ImageBarrier&;

	void transition(vk::CommandBuffer cmd) const;
};
} // namespace bave::detail
