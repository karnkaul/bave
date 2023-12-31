#include <bave/core/error.hpp>
#include <bave/graphics/detail/descriptor_cache.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave::detail {
namespace {
auto make_descriptor_pool(vk::Device device) -> vk::UniqueDescriptorPool {
	static constexpr std::uint32_t descriptor_count_v{16};
	static constexpr std::uint32_t max_sets_v{256};

	auto const pool_sizes = std::array{
		vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, descriptor_count_v},
		vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, descriptor_count_v},
		vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, descriptor_count_v},
	};

	auto dpci = vk::DescriptorPoolCreateInfo{};
	dpci.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	dpci.maxSets = max_sets_v;
	dpci.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
	dpci.pPoolSizes = pool_sizes.data();
	return device.createDescriptorPoolUnique(dpci);
}
} // namespace

DescriptorCache::DescriptorCache(NotNull<RenderDevice const*> render_device) : m_render_device(render_device) {}

auto DescriptorCache::try_allocate(vk::DescriptorSetLayout const& layout, vk::DescriptorSet& out) const -> bool {
	auto const& data = m_data.at(m_render_device->get_frame_index());
	if (!data.active) { return false; }
	auto dsai = vk::DescriptorSetAllocateInfo{};
	dsai.descriptorPool = *data.active;
	dsai.pSetLayouts = &layout;
	dsai.descriptorSetCount = 1;
	auto res = m_render_device->get_device().allocateDescriptorSets(&dsai, &out);
	return res == vk::Result::eSuccess;
}

auto DescriptorCache::allocate(vk::DescriptorSetLayout const& layout) -> vk::DescriptorSet {
	static constexpr auto max_loops{5};
	auto ret = vk::DescriptorSet{};
	auto& data = m_data.at(m_render_device->get_frame_index());
	for (int i = 0; i < max_loops; ++i) {
		if (try_allocate(layout, ret)) { return ret; }
		if (data.active) { data.used.push_back(std::move(data.active)); }
		if (data.free.empty()) { data.free.push_back(make_descriptor_pool(m_render_device->get_device())); }
		data.active = std::move(data.free.back());
		data.free.pop_back();
	}
	throw Error{"Failed to allocate Vulkan Descriptor Set"};
}

auto DescriptorCache::next_frame() -> void {
	auto& data = m_data.at(m_render_device->get_frame_index());
	if (data.active) { data.used.push_back(std::move(data.active)); }
	for (auto& pool : data.used) { m_render_device->get_device().resetDescriptorPool(*pool); }
	std::move(data.used.begin(), data.used.end(), std::back_inserter(data.free));
	data.used.clear();
	if (data.free.empty()) { data.free.push_back(make_descriptor_pool(m_render_device->get_device())); }
	std::swap(data.active, data.free.back());
	data.free.pop_back();
}

auto DescriptorCache::clear() -> void { m_data = {}; }
} // namespace bave::detail
