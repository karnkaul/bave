#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <bave/core/error.hpp>
#include <bave/desktop_app.hpp>
#include <platform/desktop/clap/clap.hpp>
#include <platform/desktop/desktop_data_store.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace bave {
namespace fs = std::filesystem;

class FileLogger {
  public:
	FileLogger(FileLogger const&) = delete;
	FileLogger(FileLogger&&) = delete;
	auto operator=(FileLogger const&) -> FileLogger& = delete;
	auto operator=(FileLogger&&) -> FileLogger& = delete;

	explicit FileLogger(std::string path) : m_path(std::move(path)), m_thread(std::thread{&FileLogger::update, this}) {}

	~FileLogger() {
		m_stop = true;
		m_thread.join();
		drain();
	}

	void push(std::string_view line) {
		auto lock = std::scoped_lock{m_mutex};
		m_buffer += line;
	}

  private:
	void update() {
		create_log_file();

		while (!m_stop) {
			std::this_thread::sleep_for(5ms);
			auto lock = std::scoped_lock{m_mutex};
			if (m_buffer.empty()) { continue; }
			drain();
		}
	}

	void create_log_file() const {
		if (fs::exists(m_path)) {
			auto const old_path = [&] {
				auto prev_file = m_path.stem();
				prev_file += ".previous";
				prev_file += m_path.extension();
				auto ret = m_path;
				ret.replace_filename(prev_file);
				return ret;
			}();
			if (fs::exists(old_path)) { fs::remove(old_path); }
			fs::rename(m_path, old_path);
		}
	}

	void drain() {
		if (auto file = std::ofstream{m_path, std::ios::app}) {
			file << m_buffer;
			m_buffer.clear();
		}
	}

	fs::path m_path{};
	std::mutex m_mutex{};
	std::string m_buffer{};
	std::thread m_thread{};
	std::atomic<bool> m_stop{};
};

auto g_file_logger = std::unique_ptr<FileLogger>{}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void log::internal::log_message(char level, CString tag, CString message) {
	auto const formatted = format_full(level, tag, message);

	auto& out = level == error_v ? std::cerr : std::cout;
	out << formatted.c_str();

	if (g_file_logger) { g_file_logger->push(formatted); }

#if defined(_WIN32)
	OutputDebugStringA(formatted.c_str());
#endif
}

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

void DesktopApp::LogFile::Deleter::operator()(LogFile const& /*log_file*/) const noexcept { g_file_logger.reset(); }

void DesktopApp::Glfw::Deleter::operator()(Glfw /*glfw*/) const noexcept { glfwTerminate(); }

void DesktopApp::Glfw::Deleter::operator()(Ptr<GLFWwindow> window) const noexcept { glfwDestroyWindow(window); }

DesktopApp::DesktopApp(CreateInfo create_info) : App("DesktopApp"), m_create_info(std::move(create_info)) {
	g_file_logger = std::make_unique<FileLogger>("bave.log");
	m_log_file.get().init = true;
	if (!m_create_info.select_gpu) {
		m_create_info.select_gpu = [](std::span<Gpu const> gpus) { return gpus.front(); };
	}
}

auto DesktopApp::do_run() -> ErrCode {
	auto options = clap::Options{
		clap::make_app_name(m_create_info.args.front()),
		"bave app",
		to_string(build_version_v),
	};
	auto const result = options.parse(m_create_info.args.subspan(1));
	if (clap::should_quit(result)) { return static_cast<ErrCode>(clap::return_code(result)); }

	m_active_pointers.emplace_back();

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
	start_next_frame(); // clear dt

	while (!is_shutting_down()) {
		start_next_frame();
		poll_events();
		tick();
		render();
	}

	m_render_device->get_device().waitIdle();

	return ErrCode::eSuccess;
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

auto DesktopApp::do_get_renderer() const -> Renderer& {
	if (!m_renderer) { throw Error{"Dereferencing null FrameRenderer"}; }
	return *m_renderer;
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
	auto assets_path = find_super_dir(m_create_info.args.front(), m_create_info.assets_patterns);
	if (assets_path.empty()) {
		m_log.error("could not locate assets via patterns: '{}'", m_create_info.assets_patterns);
		assets_path = fs::current_path().generic_string();
	}
	auto data_store = std::make_unique<DesktopDataStore>(std::move(assets_path));
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
	glfwSetCursorPosCallback(m_window.get(), [](Ptr<GLFWwindow> window, double x, double y) {
		auto& primary = self(window).m_active_pointers[0];
		primary.position = self(window).screen_to_framebuffer({x, y});
		push(window, PointerMove{.pointer = primary});
	});
	glfwSetScrollCallback(m_window.get(), [](Ptr<GLFWwindow> window, double x, double y) { push(window, MouseScroll{.delta = {x, y}}); });
	glfwSetMouseButtonCallback(m_window.get(), [](Ptr<GLFWwindow> window, int button, int action, int mods) {
		auto const& primary = self(window).m_active_pointers[0];
		push(window, PointerTap{.pointer = primary, .action = to_action(action), .mods = to_mods(mods), .button = static_cast<MouseButton>(button)});
	});

	glfwSetDropCallback(m_window.get(), [](Ptr<GLFWwindow> window, int count, char const* paths[]) { // NOLINT
		for (auto const* path : std::span{paths, static_cast<size_t>(count)}) { self(window).push_drop(path); }
	});
}

void DesktopApp::init_graphics() {
	m_render_device = std::make_unique<RenderDevice>(this);
	m_renderer = std::make_unique<Renderer>(m_render_device.get(), &get_data_store());
	m_dear_imgui = std::make_unique<detail::DearImGui>(m_window.get(), *m_render_device, m_renderer->get_render_pass());
}

void DesktopApp::poll_events() {
	glfwPollEvents();
	m_gesture_recognizer.update(get_active_pointers());
	m_game->handle_events(get_events());
	for (auto const& drop : get_file_drops()) { m_game->on_drop(drop); }
}

void DesktopApp::tick() {
	m_dear_imgui->new_frame();
	get_audio_streamer().tick(get_dt());
	if (is_shutting_down()) { return; }
	m_game->tick();
}

void DesktopApp::render() {
	m_dear_imgui->end_frame();
	if (m_renderer->start_render(m_game->clear_colour)) {
		m_game->render();
		m_dear_imgui->render(m_renderer->get_command_buffer());
	}
	m_renderer->finish_render();
}
} // namespace bave
