#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <bave/core/error.hpp>
#include <bave/detail/dear_imgui.hpp>
#include <bave/graphics/detail/command_buffer.hpp>
#include <bave/graphics/render_device.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/mat4x4.hpp>

namespace bave::detail {
void DearImGui::Instance::Deleter::operator()(Instance /*instance*/) const noexcept {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

DearImGui::DearImGui(Ptr<GLFWwindow> window, RenderDevice& render_device, vk::RenderPass render_pass) : m_blocker(render_device.get_device()) {
	static constexpr std::uint32_t max_textures_v{16};
	auto const pool_sizes = std::array{
		vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, max_textures_v},
	};
	auto dpci = vk::DescriptorPoolCreateInfo{};
	dpci.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	dpci.maxSets = max_textures_v;
	dpci.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
	dpci.pPoolSizes = pool_sizes.data();
	m_pool = render_device.get_device().createDescriptorPoolUnique(dpci);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	ImGui::StyleColorsDark();
	for (auto& colour : ImGui::GetStyle().Colors) {
		// NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
		auto const corrected = glm::convertSRGBToLinear(glm::vec4{colour.x, colour.y, colour.z, colour.w});
		colour = {corrected.x, corrected.y, corrected.z, corrected.w};
	}

	auto loader = vk::DynamicLoader{};
	auto get_fn = [&loader](char const* name) { return loader.getProcAddress<PFN_vkVoidFunction>(name); };
	auto lambda = +[](char const* name, void* ud) {
		if (std::string_view{name} == "vkCmdBeginRenderingKHR") { name = "vkCmdBeginRendering"; }
		if (std::string_view{name} == "vkCmdEndRenderingKHR") { name = "vkCmdEndRendering"; }
		auto const* gf = reinterpret_cast<decltype(get_fn)*>(ud); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		return (*gf)(name);
	};
	ImGui_ImplVulkan_LoadFunctions(lambda, &get_fn);
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = render_device.get_instance();
	init_info.PhysicalDevice = render_device.get_gpu().device;
	init_info.Device = render_device.get_device();
	init_info.QueueFamily = render_device.get_gpu().queue_family;
	init_info.Queue = render_device.get_queue();
	init_info.DescriptorPool = *m_pool;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 2;
	init_info.MSAASamples = static_cast<VkSampleCountFlagBits>(render_device.get_sample_count());
	init_info.ColorAttachmentFormat = static_cast<VkFormat>(render_device.get_swapchain_format());

	ImGui_ImplVulkan_Init(&init_info, render_pass);

	auto command_buffer = detail::CommandBuffer{render_device};
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer.get());
	command_buffer.submit(render_device);
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	m_instance = Instance{.init = true};
}

auto DearImGui::new_frame() -> void { // NOLINT(misc-no-recursion)
	if (m_state == State::eEndFrame) { end_frame(); }
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	m_state = State::eEndFrame;
}

auto DearImGui::end_frame() -> void { // NOLINT(misc-no-recursion)
	// ImGui::Render calls ImGui::EndFrame
	if (m_state == State::eNewFrame) { new_frame(); }
	ImGui::Render();
	m_state = State::eNewFrame;
}

auto DearImGui::render(vk::CommandBuffer const command_buffer) -> void {
	if (m_state == State::eEndFrame) { end_frame(); }
	if (auto* data = ImGui::GetDrawData()) { ImGui_ImplVulkan_RenderDrawData(data, command_buffer); }
}
} // namespace bave::detail
