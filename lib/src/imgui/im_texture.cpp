#include <backends/imgui_impl_vulkan.h>
#include <bave/graphics/render_device.hpp>
#include <bave/imgui/im_texture.hpp>
#include <bave/platform.hpp>

namespace bave {
void ImTexture::Deleter::operator()(vk::DescriptorSet descriptor_set) const noexcept {
	if (!descriptor_set || !device) { return; }
	if constexpr (imgui_v) {
		device.waitIdle();
		ImGui_ImplVulkan_RemoveTexture(descriptor_set);
	}
}

ImTexture::ImTexture(NotNull<std::shared_ptr<Texture>> const& texture)
	: m_texture(texture), m_descriptor_set(vk::DescriptorSet{}, Deleter{m_texture->get_image()->get_render_device().get_device()}) {
	if constexpr (imgui_v) {
		auto const cis = m_texture->combined_image_sampler();
		m_descriptor_set.get() = ImGui_ImplVulkan_AddTexture(cis.sampler, cis.image_view, static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
	}
}
} // namespace bave
