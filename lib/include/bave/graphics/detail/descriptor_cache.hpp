#pragma once
#include <bave/graphics/render_device.hpp>
#include <vulkan/vulkan.hpp>

namespace bave::detail {
class DescriptorCache {
  public:
	explicit DescriptorCache(NotNull<RenderDevice const*> render_device);

	[[nodiscard]] auto allocate(vk::DescriptorSetLayout const& layout) -> vk::DescriptorSet;

	auto next_frame() -> void;
	auto clear() -> void;

  private:
	auto try_allocate(vk::DescriptorSetLayout const& layout, vk::DescriptorSet& out) const -> bool;

	struct Data {
		std::vector<vk::UniqueDescriptorPool> used{};
		std::vector<vk::UniqueDescriptorPool> free{};
		vk::UniqueDescriptorPool active{};
	};

	NotNull<RenderDevice const*> m_render_device;
	Buffered<Data> m_data{};
};
} // namespace bave::detail
