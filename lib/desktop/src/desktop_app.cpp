#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <bave/core/error.hpp>
#include <bave/desktop_app.hpp>
#include <src/desktop_data_store.hpp>
#include <cassert>

namespace bave {
namespace {
constexpr auto to_action(int const glfw_action) {
	switch (glfw_action) {
	case GLFW_PRESS: return Action::ePress;
	case GLFW_RELEASE: return Action::eRelease;
	case GLFW_REPEAT: return Action::eRepeat;
	default: break;
	}

	return Action::eNone;
}

constexpr auto to_mods(int const glfw_mods) {
	auto ret = KeyMods{};
	if ((glfw_mods & GLFW_MOD_SHIFT) != 0) { ret.set(mod::shift); }
	if ((glfw_mods & GLFW_MOD_CONTROL) != 0) { ret.set(mod::ctrl); }
	if ((glfw_mods & GLFW_MOD_CONTROL) != 0) { ret.set(mod::ctrl); }
	if ((glfw_mods & GLFW_MOD_ALT) != 0) { ret.set(mod::alt); }
	if ((glfw_mods & GLFW_MOD_SUPER) != 0) { ret.set(mod::super); }
	if ((glfw_mods & GLFW_MOD_CAPS_LOCK) != 0) { ret.set(mod::capslock); }
	if ((glfw_mods & GLFW_MOD_NUM_LOCK) != 0) { ret.set(mod::numlock); }
	return ret;
}
} // namespace

void DesktopApp::Glfw::Deleter::operator()(Glfw /*glfw*/) const noexcept { glfwTerminate(); }

void DesktopApp::Glfw::Deleter::operator()(Ptr<GLFWwindow> window) const noexcept { glfwDestroyWindow(window); }

DesktopApp::DesktopApp(CreateInfo create_info) : App("DesktopApp"), m_create_info(std::move(create_info)) {
	if (!m_create_info.select_gpu) {
		m_create_info.select_gpu = [](std::span<Gpu const> gpus) { return gpus.front(); };
	}
}

void DesktopApp::do_run() {
	m_log.debug("glfwInit");
	glfwInit();
	m_glfw = {Glfw{.init = true}};

	glfwSetErrorCallback([](int error_code, char const* description) {
		static auto const logger = Logger{"glfw"};
		logger.error("[error {}] {}", error_code, description);
	});

	m_log.debug("init_data_store");
	init_data_store();
	m_log.debug("make_window");
	make_window();
	m_log.debug("init_graphics");
	init_graphics();
	m_game = make_game();

	while (!is_shutting_down()) {
		start_next_frame();
		poll_events();
		tick();
		render();
	}

	m_render_device->get_device().waitIdle();
}

void DesktopApp::do_shutdown() {
	m_game->shutdown();
	glfwSetWindowShouldClose(m_window.get(), GLFW_TRUE);
}

auto DesktopApp::do_get_window_size() const -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfwGetWindowSize(m_window.get(), &ret.x, &ret.y);
	return ret;
}

auto DesktopApp::do_get_framebuffer_size() const -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfwGetFramebufferSize(m_window.get(), &ret.x, &ret.y);
	return ret;
}

auto DesktopApp::do_get_render_device() const -> RenderDevice& {
	if (!m_render_device) { throw Error{"Dereferencing null RenderDevice"}; }
	return *m_render_device;
}

auto DesktopApp::do_get_frame_renderer() const -> FrameRenderer& {
	if (!m_frame_renderer) { throw Error{"Dereferencing null FrameRenderer"}; }
	return *m_frame_renderer;
}

auto DesktopApp::get_instance_extensions() const -> std::span<char const* const> {
	auto count = std::uint32_t{};
	auto const* const extensions_array = glfwGetRequiredInstanceExtensions(&count);
	return std::span{extensions_array, count};
}

auto DesktopApp::make_surface(vk::Instance instance) const -> vk::SurfaceKHR {
	VkSurfaceKHR ret{};
	glfwCreateWindowSurface(instance, m_window.get(), {}, &ret);
	return vk::SurfaceKHR{ret};
}

auto DesktopApp::get_framebuffer_extent() const -> vk::Extent2D {
	auto const size = glm::uvec2{get_framebuffer_size()};
	return {size.x, size.y};
}

auto DesktopApp::select_gpu(std::span<Gpu const> gpus) const -> Gpu {
	if (m_create_info.select_gpu) { return m_create_info.select_gpu(gpus); }
	return IWsi::select_gpu(gpus);
}

auto DesktopApp::self(Ptr<GLFWwindow> window) -> DesktopApp& {
	auto* ret = static_cast<DesktopApp*>(glfwGetWindowUserPointer(window));
	if (ret == nullptr) { throw Error{"Dereferencing null GLFW Window User Pointer"}; }
	return *ret;
}

void DesktopApp::push(Ptr<GLFWwindow> window, Event event) { self(window).push_event(event); }

void DesktopApp::init_data_store() {
	auto data_path = DesktopDataStore::find_super_dir(m_create_info.args.front(), m_create_info.assets_pattern);
	auto data_store = std::make_unique<DesktopDataStore>(std::move(data_path));
	set_data_store(std::move(data_store));
}

void DesktopApp::make_window() {
	if (glfwVulkanSupported() == GLFW_FALSE) { throw Error{"Vulkan not supported"}; }

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	auto* window = glfwCreateWindow(m_create_info.extent.x, m_create_info.extent.y, m_create_info.title.c_str(), nullptr, nullptr);
	m_window = std::unique_ptr<GLFWwindow, Glfw::Deleter>{window};
	if (m_window == nullptr) { throw Error{"Failed to create Window"}; }
	glfwSetWindowUserPointer(m_window.get(), this);

	if (m_create_info.lock_aspect_ratio) { glfwSetWindowAspectRatio(m_window.get(), m_create_info.extent.x, m_create_info.extent.y); }

	glfwSetWindowCloseCallback(m_window.get(), [](Ptr<GLFWwindow> window) { self(window).shutdown(); });
	glfwSetWindowFocusCallback(m_window.get(), [](Ptr<GLFWwindow> window, int v) { push(window, FocusChange{.in_focus = v == GLFW_TRUE}); });
	glfwSetWindowSizeCallback(m_window.get(), [](Ptr<GLFWwindow> window, int x, int y) { push(window, WindowResize{.extent = {x, y}}); });
	glfwSetFramebufferSizeCallback(m_window.get(), [](Ptr<GLFWwindow> window, int x, int y) { push(window, FramebufferResize{.extent = {x, y}}); });
	glfwSetKeyCallback(m_window.get(), [](Ptr<GLFWwindow> window, int key, int scancode, int action, int mods) {
		push(window, KeyInput{.key = static_cast<Key>(key), .action = to_action(action), .mods = to_mods(mods), .scancode = scancode});
	});
	glfwSetCharCallback(m_window.get(), [](Ptr<GLFWwindow> window, std::uint32_t code) { push(window, CharInput{.code = code}); });
	glfwSetCursorPosCallback(m_window.get(), [](Ptr<GLFWwindow> window, double x, double y) { push(window, CursorMove{.position = {x, y}}); });
	glfwSetScrollCallback(m_window.get(), [](Ptr<GLFWwindow> window, double x, double y) { push(window, MouseScroll{.delta = {x, y}}); });
	glfwSetMouseButtonCallback(m_window.get(), [](Ptr<GLFWwindow> window, int button, int action, int mods) {
		auto position = glm::dvec2{};
		glfwGetCursorPos(window, &position.x, &position.y);
		push(window, MouseClick{.id = button, .action = to_action(action), .mods = to_mods(mods), .position = position});
	});
}

void DesktopApp::init_graphics() {
	m_render_device = std::make_unique<RenderDevice>(this);
	m_frame_renderer = std::make_unique<FrameRenderer>(m_render_device.get(), &get_data_store());
	m_dear_imgui = std::make_unique<DearImGui>(m_window.get(), *m_render_device, m_frame_renderer->get_render_pass());
}

void DesktopApp::poll_events() { glfwPollEvents(); }

void DesktopApp::tick() {
	m_dear_imgui->new_frame();
	if (is_shutting_down()) { return; }
	assert(m_game != nullptr);
	m_game->tick();
}

void DesktopApp::render() {
	m_dear_imgui->end_frame();
	auto command_buffer = m_frame_renderer->start_render(m_game->clear_colour);
	if (command_buffer) {
		m_game->render(command_buffer);
		m_dear_imgui->render(command_buffer);
	}
	m_frame_renderer->finish_render();
}
} // namespace bave
