#include <bave/core/error.hpp>
#include <bave/graphics/command_buffer.hpp>
#include <bave/graphics/image_barrier.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/render_resource.hpp>
#include <bave/graphics/utils.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace bave {
namespace {
struct VmaImage {
	VmaAllocation allocation{};
	vk::Image image{};
	vk::UniqueImageView image_view{};

	static auto make(RenderDevice& render_device, RenderImage::CreateInfo const& m_create_info, vk::Extent2D extent, std::uint32_t mip_levels = 1) {
		auto ret = VmaImage{};
		auto vaci = VmaAllocationCreateInfo{};
		vaci.usage = VMA_MEMORY_USAGE_AUTO;
		auto ici = vk::ImageCreateInfo{};
		ici.usage = m_create_info.usage;
		ici.imageType = vk::ImageType::e2D;
		ici.tiling = m_create_info.tiling;
		ici.arrayLayers = m_create_info.view_type == vk::ImageViewType::eCube ? RenderImage::cubemap_layers_v : 1;
		if (ici.arrayLayers == RenderImage::cubemap_layers_v) { ici.flags |= vk::ImageCreateFlagBits::eCubeCompatible; }
		ici.mipLevels = mip_levels;
		ici.extent = vk::Extent3D{extent, 1};
		ici.format = m_create_info.format;
		ici.samples = m_create_info.samples;
		auto const vici = static_cast<VkImageCreateInfo>(ici);

		auto* image = VkImage{};
		if (vmaCreateImage(render_device.get_allocator(), &vici, &vaci, &image, &ret.allocation, {}) != VK_SUCCESS) {
			throw Error{"Failed to allocate Vulkan Image"};
		}
		ret.image = image;

		auto const isr = vk::ImageSubresourceRange{m_create_info.aspect, 0, ici.mipLevels, 0, ici.arrayLayers};
		auto const make_image_view = MakeImageView{.image = ret.image, .format = ici.format, .subresource = isr, .type = m_create_info.view_type};
		ret.image_view = make_image_view(render_device.get_device());

		return ret;
	}
};

struct MipMapWriter {
	// NOLINTNEXTLINE
	ImageBarrier& ib;

	vk::Extent2D extent;
	vk::CommandBuffer cb;
	std::uint32_t mip_levels;

	std::uint32_t layer_count{1};

	auto blit_mips(std::uint32_t const src_level, vk::Offset3D const src_offset, vk::Offset3D const dst_offset) const -> void {
		auto const src_isrl = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, src_level, 0, layer_count};
		auto const dst_isrl = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, src_level + 1, 0, layer_count};
		auto const region = vk::ImageBlit{src_isrl, {vk::Offset3D{}, src_offset}, dst_isrl, {vk::Offset3D{}, dst_offset}};
		cb.blitImage(ib.barrier.image, vk::ImageLayout::eTransferSrcOptimal, ib.barrier.image, vk::ImageLayout::eTransferDstOptimal, region,
					 vk::Filter::eLinear);
	}

	auto blit_next_mip(std::uint32_t const src_level, vk::Offset3D const src_offset, vk::Offset3D const dst_offset) -> void {
		ib.barrier.oldLayout = vk::ImageLayout::eUndefined;
		ib.barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		ib.barrier.subresourceRange.baseMipLevel = src_level + 1;
		ib.transition(cb);

		blit_mips(src_level, src_offset, dst_offset);

		ib.barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		ib.barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		ib.transition(cb);
	}

	auto operator()() -> void {
		ib.barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		ib.barrier.subresourceRange.baseArrayLayer = 0;
		ib.barrier.subresourceRange.layerCount = layer_count;
		ib.barrier.subresourceRange.baseMipLevel = 0;
		ib.barrier.subresourceRange.levelCount = 1;

		ib.barrier.srcAccessMask = ib.barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite;
		ib.src_stage = vk::PipelineStageFlagBits::eAllCommands;
		ib.dst_stage = vk::PipelineStageFlagBits::eTransfer;

		ib.barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		ib.barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		ib.barrier.subresourceRange.baseMipLevel = 0;
		ib.transition(cb);

		ib.src_stage = ib.dst_stage = vk::PipelineStageFlagBits::eTransfer;

		auto src_extent = vk::Extent3D{extent, 1};
		for (std::uint32_t mip = 0; mip + 1 < mip_levels; ++mip) {
			vk::Extent3D dst_extent = vk::Extent3D(std::max(src_extent.width / 2, 1u), std::max(src_extent.height / 2, 1u), 1u);
			auto const src_offset = vk::Offset3D{static_cast<int>(src_extent.width), static_cast<int>(src_extent.height), 1};
			auto const dst_offset = vk::Offset3D{static_cast<int>(dst_extent.width), static_cast<int>(dst_extent.height), 1};
			blit_next_mip(mip, src_offset, dst_offset);
			src_extent = dst_extent;
		}

		ib.barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		ib.barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		ib.barrier.subresourceRange.baseMipLevel = 0;
		ib.barrier.subresourceRange.levelCount = mip_levels;
		ib.dst_stage = vk::PipelineStageFlagBits::eAllCommands;
		ib.transition(cb);
	}
};

struct CopyBufferToImage {
	vk::Image target_image{};
	vk::Extent2D image_extent{};
	vk::Offset2D target_offset{};
	vk::Buffer source_bytes{};
	vk::Extent2D source_extent{};
	std::uint32_t array_layers{1};
	std::uint32_t mip_levels{1};

	auto operator()(vk::CommandBuffer cmd) const {
		auto const isrl = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, array_layers);
		auto const vk_extent = vk::Extent3D{source_extent, 1u};
		auto const bic = vk::BufferImageCopy({}, {}, {}, isrl, vk::Offset3D{target_offset, 0}, vk_extent);
		auto barrier = ImageBarrier{target_image, mip_levels, array_layers};
		barrier.set_full_barrier(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal).transition(cmd);
		cmd.copyBufferToImage(source_bytes, target_image, vk::ImageLayout::eTransferDstOptimal, bic);
		barrier.set_full_barrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal).transition(cmd);

		if (mip_levels > 1) { MipMapWriter{barrier, image_extent, cmd, mip_levels, array_layers}(); }
	}
};

struct CopyImage {
	vk::Image image{};
	vk::ImageLayout layout{};
	vk::Extent2D extent{};
	vk::Offset2D offset{};
};

struct CopyImageToImage {
	CopyImage source{};
	CopyImage target{};

	std::uint32_t array_layers{1};
	std::uint32_t mip_levels{1};

	auto operator()(vk::CommandBuffer cmd) const {
		auto const isrl = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, array_layers);
		auto source_barrier = ImageBarrier{source.image, mip_levels, array_layers};
		auto target_barrier = ImageBarrier{target.image, mip_levels, array_layers};
		source_barrier.set_full_barrier(source.layout, vk::ImageLayout::eTransferSrcOptimal).transition(cmd);
		target_barrier.set_full_barrier(target.layout, vk::ImageLayout::eTransferDstOptimal).transition(cmd);
		auto image_copy = vk::ImageCopy{isrl, vk::Offset3D{source.offset, 0}, isrl, vk::Offset3D{target.offset, 0}, vk::Extent3D{source.extent, 1}};
		cmd.copyImage(source.image, vk::ImageLayout::eTransferSrcOptimal, target.image, vk::ImageLayout::eTransferDstOptimal, image_copy);
		source_barrier.set_full_barrier(vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal).transition(cmd);
		target_barrier.set_full_barrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal).transition(cmd);

		if (mip_levels > 1) { MipMapWriter{target_barrier, target.extent, cmd, mip_levels, array_layers}(); }
	}
};
} // namespace

void RenderResource::Deleter::operator()(vk::Buffer buffer) const { vmaDestroyBuffer(allocator, buffer, allocation); }
void RenderResource::Deleter::operator()(vk::Image image) const { vmaDestroyImage(allocator, image, allocation); }

RenderBuffer::RenderBuffer(NotNull<RenderDevice*> render_device, vk::BufferUsageFlags usage, vk::DeviceSize capacity)
	: m_render_device(render_device), m_usage(usage) {
	resize(capacity);
}

auto RenderBuffer::resize(std::size_t new_capacity) -> void {
	auto vaci = VmaAllocationCreateInfo{};
	vaci.usage = VMA_MEMORY_USAGE_AUTO;
	vaci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	auto const bci = vk::BufferCreateInfo{{}, new_capacity, m_usage | vk::BufferUsageFlagBits::eTransferDst};
	auto vbci = static_cast<VkBufferCreateInfo>(bci);

	auto* allocation = VmaAllocation{};
	auto* buffer = VkBuffer{};
	auto alloc_info = VmaAllocationInfo{};
	if (vmaCreateBuffer(m_render_device->get_allocator(), &vbci, &vaci, &buffer, &allocation, &alloc_info) != VK_SUCCESS) {
		throw Error{"Failed to allocate Vulkan Buffer"};
	}

	m_buffer = {buffer, Deleter{.allocator = m_render_device->get_allocator(), .allocation = allocation}};
	m_capacity = bci.size;
	m_mapped = alloc_info.pMappedData;
	m_size = {};

	assert(m_mapped);
}

auto RenderBuffer::write(void const* data, std::size_t size) -> void {
	if (size > m_capacity) { resize(size); }
	if (size > 0) { std::memcpy(m_mapped, data, size); }
	m_size = size;
}

auto RenderImage::compute_mip_levels(vk::Extent2D extent) -> std::uint32_t {
	return static_cast<std::uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1u;
}

RenderImage::RenderImage(NotNull<RenderDevice*> render_device, CreateInfo const& info, vk::Extent2D extent) : m_render_device(render_device) {
	if (extent.width == 0) { extent.width = 1; }
	if (extent.height == 0) { extent.height = 1; }
	m_create_info = info;
	recreate(extent);
}

auto RenderImage::recreate(vk::Extent2D extent) -> void {
	if (extent.width == 0 || extent.height == 0) { return; }

	auto const mip_levels = m_create_info.mip_map ? compute_mip_levels(extent) : 1;
	auto vma_image = VmaImage::make(*m_render_device, m_create_info, extent, mip_levels);

	m_image = {vma_image.image, Deleter{.allocator = m_render_device->get_allocator(), .allocation = vma_image.allocation}};
	m_view = std::move(vma_image.image_view);
	m_extent = extent;
	m_layout = vk::ImageLayout::eUndefined;
	m_mip_levels = mip_levels;
}

auto RenderImage::copy_from(std::span<Layer const> layers, vk::Extent2D target_extent) -> bool {
	auto const array_layers = m_create_info.view_type == vk::ImageViewType::eCube ? cubemap_layers_v : 1;
	if (target_extent.width == 0 || target_extent.height == 0) { return false; }
	if (layers.size() != array_layers) { return false; }

	if (m_extent != target_extent) { recreate(target_extent); }
	auto const accumulate_size = [](std::size_t total, Layer const layer) { return total + layer.size_bytes(); };
	auto const size = std::accumulate(layers.begin(), layers.end(), std::size_t{}, accumulate_size);
	auto staging = RenderBuffer{m_render_device, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, size};

	auto* ptr = static_cast<std::byte*>(staging.get_mapped());
	for (auto const& image : layers) {
		std::memcpy(ptr, image.data(), image.size_bytes());
		ptr += image.size_bytes(); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	}

	auto cmd = CommandBuffer{*m_render_device};
	CopyBufferToImage{
		.target_image = m_image,
		.image_extent = m_extent,
		.target_offset = {},
		.source_bytes = staging.get_buffer(),
		.source_extent = target_extent,
		.array_layers = array_layers,
		.mip_levels = m_mip_levels,
	}(cmd.get());

	cmd.submit(*m_render_device);

	return true;
}

auto RenderImage::overwrite(Bitmap const& bitmap, glm::uvec2 top_left) -> bool {
	if (m_create_info.view_type == vk::ImageViewType::eCube || bitmap.extent.x == 0 || bitmap.extent.y == 0) { return false; }

	auto const overwrite_extent = bitmap.extent + top_left;
	auto const current_extent = glm::uvec2{m_extent.width, m_extent.height};
	if (overwrite_extent.x > current_extent.x || overwrite_extent.y > current_extent.y) { return false; }

	auto staging = RenderBuffer{m_render_device, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, bitmap.bytes.size_bytes()};
	std::memcpy(staging.get_mapped(), bitmap.bytes.data(), bitmap.bytes.size_bytes());

	auto cmd = CommandBuffer{*m_render_device};
	auto const offset = glm::ivec2{top_left};
	CopyBufferToImage{
		.target_image = m_image,
		.image_extent = m_extent,
		.target_offset = {offset.x, offset.y},
		.source_bytes = staging.get_buffer(),
		.source_extent = {bitmap.extent.x, bitmap.extent.y},
		.array_layers = 1,
		.mip_levels = m_mip_levels,
	}(cmd.get());

	cmd.submit(*m_render_device);

	return true;
}

auto RenderImage::overwrite(std::span<BitmapWrite const> writes) -> bool {
	if (m_create_info.view_type == vk::ImageViewType::eCube) { return false; }

	auto const accumulate_size = [](std::size_t total, BitmapWrite const& iw) { return total + iw.bitmap.bytes.size_bytes(); };
	auto const size = std::accumulate(writes.begin(), writes.end(), std::size_t{}, accumulate_size);
	auto staging = RenderBuffer{m_render_device, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, size};

	auto const isrl = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
	auto bics = std::vector<vk::BufferImageCopy>{};
	bics.reserve(writes.size());
	auto* ptr = static_cast<std::byte*>(staging.get_mapped());
	auto buffer_offset = vk::DeviceSize{};
	for (auto const& iw : writes) {
		std::memcpy(ptr + buffer_offset, iw.bitmap.bytes.data(), iw.bitmap.bytes.size_bytes()); // NOLINT
		auto const image_offset = glm::ivec2{iw.top_left};
		auto const bic = vk::BufferImageCopy{
			buffer_offset, {}, {}, isrl, vk::Offset3D{image_offset.x, image_offset.y, 0}, {iw.bitmap.extent.x, iw.bitmap.extent.y, 1},
		};
		bics.push_back(bic);
		buffer_offset += iw.bitmap.bytes.size_bytes();
	}

	auto cmd = CommandBuffer{*m_render_device};
	auto barrier = ImageBarrier{m_image, m_mip_levels, 1};
	barrier.set_full_barrier(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal).transition(cmd);
	cmd.get().copyBufferToImage(staging.get_buffer(), m_image, vk::ImageLayout::eTransferDstOptimal, bics);
	barrier.set_full_barrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal).transition(cmd);

	if (m_mip_levels > 1) { MipMapWriter{barrier, m_extent, cmd, m_mip_levels, 1}(); }

	cmd.submit(*m_render_device);

	return true;
}

auto RenderImage::resize(vk::Extent2D extent) -> void {
	assert(extent.width < 40960 && extent.height < 40960);
	auto const mip_levels = m_create_info.mip_map ? compute_mip_levels(extent) : 1;
	auto new_image = VmaImage::make(*m_render_device, m_create_info, extent, mip_levels);
	auto const copy_src = CopyImage{
		.image = m_image,
		.layout = vk::ImageLayout::eShaderReadOnlyOptimal,
		.extent = m_extent,
	};
	auto const copy_dst = CopyImage{
		.image = new_image.image,
		.layout = vk::ImageLayout::eUndefined,
		.extent = extent,
	};

	auto cmd = CommandBuffer{*m_render_device};
	CopyImageToImage{
		.source = copy_src,
		.target = copy_dst,
		.array_layers = m_create_info.view_type == vk::ImageViewType::eCube ? cubemap_layers_v : 1,
		.mip_levels = mip_levels,
	}(cmd);

	cmd.submit(*m_render_device);
}
} // namespace bave
