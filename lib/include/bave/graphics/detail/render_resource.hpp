#pragma once
#include <vk_mem_alloc.h>
#include <bave/core/not_null.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/core/scoped_resource.hpp>
#include <bave/graphics/bitmap.hpp>
#include <vulkan/vulkan.hpp>
#include <span>

namespace bave {
class RenderDevice;

namespace detail {
class RenderResource : public Polymorphic {
  protected:
	struct Deleter {
		VmaAllocator allocator{};
		VmaAllocation allocation{};

		void operator()(vk::Buffer buffer) const;
		void operator()(vk::Image image) const;
	};

	RenderResource() = default;
};

class RenderBuffer : public RenderResource {
  public:
	explicit RenderBuffer(NotNull<RenderDevice*> render_device, vk::BufferUsageFlags usage, vk::DeviceSize capacity = 1);

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return *m_render_device; }

	[[nodiscard]] auto get_buffer() const -> vk::Buffer { return m_buffer.get(); }
	[[nodiscard]] auto get_usage() const -> vk::BufferUsageFlags { return m_usage; }
	[[nodiscard]] auto get_capacity() const -> vk::DeviceSize { return m_capacity; }
	[[nodiscard]] auto get_size() const -> vk::DeviceSize { return m_size; }

	auto resize(vk::DeviceSize new_capacity) -> void;

	[[nodiscard]] auto get_mapped() const -> void const* { return m_mapped; }
	[[nodiscard]] auto get_mapped() -> void* { return m_mapped; }

	auto write(void const* data, vk::DeviceSize size) -> void;

	operator vk::Buffer() const { return get_buffer(); }

  protected:
	NotNull<RenderDevice*> m_render_device;
	ScopedResource<vk::Buffer, Deleter> m_buffer{};
	vk::BufferUsageFlags m_usage{};
	vk::DeviceSize m_capacity{};
	vk::DeviceSize m_size{};
	void* m_mapped{};
};

class RenderImage : public RenderResource {
  public:
	struct CreateInfo {
		static constexpr auto usage_v = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

		vk::Format format{vk::Format::eR8G8B8A8Srgb};
		vk::ImageUsageFlags usage{usage_v};
		vk::ImageAspectFlagBits aspect{vk::ImageAspectFlagBits::eColor};
		vk::ImageTiling tiling{vk::ImageTiling::eOptimal};
		vk::SampleCountFlagBits samples{vk::SampleCountFlagBits::e1};
		vk::ImageViewType view_type{vk::ImageViewType::e2D};
		bool mip_map{true};
	};

	using Layer = std::span<std::byte const>;

	static constexpr vk::Extent2D min_extent_v{1, 1};
	static constexpr std::uint32_t cubemap_layers_v{6};

	static auto compute_mip_levels(vk::Extent2D extent) -> std::uint32_t;

	explicit RenderImage(NotNull<RenderDevice*> render_device, CreateInfo const& create_info, vk::Extent2D extent = min_extent_v);

	auto copy_from(BitmapView bitmap) -> bool;

	void recreate(vk::Extent2D extent);
	auto overwrite(BitmapView bitmap, glm::ivec2 top_left) -> bool;
	void resize(vk::Extent2D extent);

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return *m_render_device; }

	[[nodiscard]] auto get_image() const -> vk::Image { return m_image.get(); }
	[[nodiscard]] auto get_extent() const -> vk::Extent2D { return m_extent; }
	[[nodiscard]] auto get_format() const -> vk::Format { return m_create_info.format; }
	[[nodiscard]] auto get_image_view() const -> vk::ImageView { return *m_view; }
	[[nodiscard]] auto get_view_type() const -> vk::ImageViewType { return m_create_info.view_type; }
	[[nodiscard]] auto get_layout() const -> vk::ImageLayout { return m_layout; }
	[[nodiscard]] auto get_mip_levels() const -> std::uint32_t { return m_mip_levels; }

	[[nodiscard]] auto get_create_info() const -> CreateInfo const& { return m_create_info; }

	operator vk::Image() const { return get_image(); }
	operator vk::ImageView() const { return *m_view; }

  protected:
	NotNull<RenderDevice*> m_render_device;
	CreateInfo m_create_info{};
	ScopedResource<vk::Image, Deleter> m_image{};
	vk::Extent2D m_extent{};
	vk::UniqueImageView m_view{};
	vk::ImageLayout m_layout{};
	std::uint32_t m_mip_levels{};
};

template <typename Type>
constexpr auto to_vk_extent(glm::tvec2<Type> const in) -> vk::Extent2D {
	auto const uextent = glm::uvec2{in};
	return {uextent.x, uextent.y};
}
} // namespace detail
} // namespace bave
