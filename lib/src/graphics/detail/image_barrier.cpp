#include <bave/graphics/detail/image_barrier.hpp>
#include <bave/graphics/detail/render_resource.hpp>

namespace bave::detail {
ImageBarrier::ImageBarrier(vk::Image image, std::uint32_t mip_levels, std::uint32_t array_layers) {
	barrier.image = image;
	barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, mip_levels, 0, array_layers};
}

ImageBarrier::ImageBarrier(RenderImage const& image)
	: ImageBarrier(image.get_image(), image.get_mip_levels(), image.get_view_type() == vk::ImageViewType::eCube ? RenderImage::cubemap_layers_v : 1) {}

auto ImageBarrier::set_full_barrier(vk::ImageLayout src, vk::ImageLayout dst) -> ImageBarrier& {
	barrier.oldLayout = src;
	barrier.newLayout = dst;
	src_stage = dst_stage = vk::PipelineStageFlagBits::eAllCommands;
	barrier.srcAccessMask = barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite;
	return *this;
}

auto ImageBarrier::set_undef_to_optimal(bool depth) -> ImageBarrier& {
	barrier.oldLayout = vk::ImageLayout::eUndefined;
	src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
	barrier.srcAccessMask = vk::AccessFlagBits::eNone;
	if (depth) {
		dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
		barrier.newLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	} else {
		dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	}
	return *this;
}

auto ImageBarrier::set_undef_to_transfer_dst() -> ImageBarrier& {
	barrier.oldLayout = vk::ImageLayout::eUndefined;
	barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
	src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
	barrier.srcAccessMask = vk::AccessFlagBits::eNone;
	dst_stage = vk::PipelineStageFlagBits::eTransfer;
	barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eTransferRead;
	return *this;
}

auto ImageBarrier::set_optimal_to_read_only(bool depth) -> ImageBarrier& {
	barrier.newLayout = vk::ImageLayout::eReadOnlyOptimal;
	if (depth) {
		src_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		barrier.oldLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	} else {
		src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
		barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	}
	dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	return *this;
}

auto ImageBarrier::set_optimal_to_transfer_src() -> ImageBarrier& {
	barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
	dst_stage = vk::PipelineStageFlagBits::eTransfer;
	barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
	return *this;
}

auto ImageBarrier::set_optimal_to_present() -> ImageBarrier& {
	barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
	src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
	dst_stage = vk::PipelineStageFlagBits::eBottomOfPipe;
	barrier.dstAccessMask = vk::AccessFlagBits::eNone;
	return *this;
}

auto ImageBarrier::set_transfer_dst_to_optimal(bool depth) -> ImageBarrier& {
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	src_stage = vk::PipelineStageFlagBits::eTransfer;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eTransferRead;
	if (depth) {
		dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		barrier.newLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	} else {
		dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	}
	return *this;
}

auto ImageBarrier::set_transfer_dst_to_present() -> ImageBarrier& {
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
	src_stage = vk::PipelineStageFlagBits::eTransfer;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eTransferRead;
	dst_stage = vk::PipelineStageFlagBits::eBottomOfPipe;
	barrier.dstAccessMask = vk::AccessFlagBits::eNone;
	return *this;
}

auto ImageBarrier::transition(vk::CommandBuffer cmd) const -> void {
	if (!barrier.image) { return; }
	cmd.pipelineBarrier(src_stage, dst_stage, vk::DependencyFlags{}, {}, {}, barrier);
}
} // namespace bave::detail
