#include <GLFW/glfw3.h>
#include <bave/core/error.hpp>
#include <bave/desktop_app.hpp>
#include <cassert>

namespace bave {
namespace {
struct ScopedGlfw {
	ScopedGlfw(ScopedGlfw const&) = delete;
	ScopedGlfw(ScopedGlfw&&) = delete;
	auto operator=(ScopedGlfw const&) -> ScopedGlfw& = delete;
	auto operator=(ScopedGlfw&&) -> ScopedGlfw& = delete;

	ScopedGlfw() { glfwInit(); }
	~ScopedGlfw() noexcept { glfwTerminate(); }
};

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

DesktopApp::DesktopApp(CreateInfo const& create_info) : App("DesktopApp"), m_create_info(create_info) {}

void DesktopApp::do_run() {
	auto glfw = ScopedGlfw{};

	make_window();

	auto delta_time = DeltaTime{};
	while (!is_shutting_down()) {
		m_dt = delta_time.update();
		poll_events();
		tick();
		render();
	}
}

void DesktopApp::do_shutdown() { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }

auto DesktopApp::do_get_window_size() const -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfwGetWindowSize(m_window, &ret.x, &ret.y);
	return ret;
}

auto DesktopApp::do_get_framebuffer_size() const -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfwGetFramebufferSize(m_window, &ret.x, &ret.y);
	return ret;
}

auto DesktopApp::self(Ptr<GLFWwindow> window) -> DesktopApp& {
	auto* ret = static_cast<DesktopApp*>(glfwGetWindowUserPointer(window));
	if (ret == nullptr) { throw Error{"Dereferencing null GLFW Window User Pointer"}; }
	return *ret;
}

void DesktopApp::push(Ptr<GLFWwindow> window, Event event) { self(window).m_events.push_back(event); }

void DesktopApp::make_window() {
	if (glfwVulkanSupported() == GLFW_FALSE) { throw Error{"Vulkan not supported"}; }

	m_window = glfwCreateWindow(m_create_info.extent.x, m_create_info.extent.y, m_create_info.title.c_str(), nullptr, nullptr);
	if (m_window == nullptr) { throw Error{"Failed to create Window"}; }
	glfwSetWindowUserPointer(m_window, this);

	glfwSetWindowCloseCallback(m_window, [](Ptr<GLFWwindow> window) { self(window).shutdown(); });
	glfwSetWindowFocusCallback(m_window, [](Ptr<GLFWwindow> window, int v) { push(window, FocusChange{.in_focus = v == GLFW_TRUE}); });
	glfwSetWindowSizeCallback(m_window, [](Ptr<GLFWwindow> window, int x, int y) { push(window, WindowResize{.extent = {x, y}}); });
	glfwSetFramebufferSizeCallback(m_window, [](Ptr<GLFWwindow> window, int x, int y) { push(window, FramebufferResize{.extent = {x, y}}); });
	glfwSetKeyCallback(m_window, [](Ptr<GLFWwindow> window, int key, int scancode, int action, int mods) {
		push(window, KeyInput{.key = static_cast<Key>(key), .action = to_action(action), .mods = to_mods(mods), .scancode = scancode});
	});
	glfwSetCharCallback(m_window, [](Ptr<GLFWwindow> window, std::uint32_t code) { push(window, CharInput{.code = code}); });
	glfwSetCursorPosCallback(m_window, [](Ptr<GLFWwindow> window, double x, double y) { push(window, CursorMove{.position = {x, y}}); });
	glfwSetScrollCallback(m_window, [](Ptr<GLFWwindow> window, double x, double y) { push(window, MouseScroll{.delta = {x, y}}); });
	glfwSetMouseButtonCallback(m_window, [](Ptr<GLFWwindow> window, int button, int action, int mods) {
		auto position = glm::dvec2{};
		glfwGetCursorPos(window, &position.x, &position.y);
		push(window, MouseClick{.id = button, .action = to_action(action), .mods = to_mods(mods), .position = position});
	});
}

void DesktopApp::poll_events() {
	m_events.clear();
	glfwPollEvents();
}

void DesktopApp::tick() {
	if (is_shutting_down()) { return; }
	assert(m_game != nullptr);
	m_game->tick();
}

void DesktopApp::render() {}
} // namespace bave
