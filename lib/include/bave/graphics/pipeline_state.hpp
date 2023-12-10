#pragma once
#include <vulkan/vulkan.hpp>

namespace bave {
struct PipelineState {
	vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
	vk::CompareOp depth_compare{vk::CompareOp::eLess};
	vk::Bool32 depth_test_write{vk::True};
	float line_width{1.0f};
};
} // namespace bave
