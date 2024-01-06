#include <backends/imgui_impl_vulkan.h>
#include <bave/graphics/render_device.hpp>
#include <bave/im_texture.hpp>
#include <bave/platform.hpp>
#include <cassert>

namespace bave {
void ImTexture::Deleter::operator()(vk::DescriptorSet descriptor_set) const noexcept {
	if constexpr (imgui_v) {
		// assert(render_device != nullptr);
		// render_device->get_defer_queue().push([descriptor_set] { ImGui_ImplVulkan_RemoveTexture(descriptor_set); });
		ImGui_ImplVulkan_RemoveTexture(descriptor_set);
	}
}

ImTexture::ImTexture(NotNull<std::shared_ptr<Texture>> const& texture) : m_texture(texture) {
	if constexpr (imgui_v) {
		auto const cis = m_texture->combined_image_sampler();
		m_descriptor_set.get() = ImGui_ImplVulkan_AddTexture(cis.sampler, cis.image_view, static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
	}
}
} // namespace bave
