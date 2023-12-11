#include <android_native_app_glue.h>
#include <jni.h>
#include <bave/android_app.hpp>
#include <bave/core/error.hpp>
#include <bave/core/ptr.hpp>
#include <bave/game.hpp>
#include <src/android_data_store.hpp>
#include <cassert>

// temp
#include <EGL/egl.h>
#include <GLES/gl.h>

namespace bave {
namespace {
constexpr auto to_key(int const key) {
	switch (key) {
	case AKEYCODE_BACK:
	case AKEYCODE_ESCAPE: return Key::eEscape;
	case AKEYCODE_W: return Key::eW;
	default: return Key::eUnknown;
	}
}

constexpr auto to_key_action(int const action) {
	switch (action) {
	case AKEY_EVENT_ACTION_DOWN: return Action::ePress;
	case AKEY_EVENT_ACTION_UP: return Action::eRelease;
	default: return Action::eNone;
	}
}

constexpr auto to_key_mods(int const mods) {
	auto ret = KeyMods{};
	if ((mods & AMETA_CTRL_ON) != 0) { ret.set(mod::ctrl); }
	if ((mods & AMETA_SHIFT_ON) != 0) { ret.set(mod::shift); }
	if ((mods & AMETA_ALT_ON) != 0) { ret.set(mod::alt); }
	if ((mods & AMETA_CAPS_LOCK_ON) != 0) { ret.set(mod::capslock); }
	if ((mods & AMETA_NUM_LOCK_ON) != 0) { ret.set(mod::numlock); }
	return ret;
}
} // namespace

AndroidApp::AndroidApp(android_app& app) : m_app(app) { m_app.userData = this; }

void AndroidApp::do_run() {
	app_dummy(); // NOLINT

	setup_event_callbacks();

	while (!m_app.destroyRequested) {
		start_next_frame();
		poll_events();
		tick();
		render();
	}
}

void AndroidApp::do_shutdown() { ANativeActivity_finish(m_app.activity); }

auto AndroidApp::do_get_framebuffer_size() const -> glm::ivec2 {
	if (!m_app.window) { return {}; }
	return {ANativeWindow_getWidth(m_app.window), ANativeWindow_getHeight(m_app.window)};
}

auto AndroidApp::do_get_render_device() const -> RenderDevice& {
	if (!m_render_device) { throw Error{"Dereferencing null RenderDevice"}; }
	return *m_render_device;
}

auto AndroidApp::do_get_frame_renderer() const -> FrameRenderer& {
	if (!m_frame_renderer) { throw Error{"Dereferencing null FrameRenderer"}; }
	return *m_frame_renderer;
}

auto AndroidApp::get_instance_extensions() const -> std::span<char const* const> {
	static constexpr auto ret = std::array{
		"VK_KHR_surface",
		"VK_KHR_android_surface",
	};
	return ret;
}

auto AndroidApp::make_surface(vk::Instance instance) const -> vk::SurfaceKHR {
	auto asci = vk::AndroidSurfaceCreateInfoKHR{};
	asci.window = m_app.window;
	return instance.createAndroidSurfaceKHR(asci);
}

auto AndroidApp::get_framebuffer_extent() const -> vk::Extent2D {
	auto const size = glm::uvec2{get_framebuffer_size()};
	return {size.x, size.y};
}

auto AndroidApp::self(Ptr<android_app> app) -> AndroidApp& {
	auto* ret = static_cast<AndroidApp*>(app->userData);
	if (ret == nullptr) { throw Error{"Dereferencing null GLFW Window User Pointer"}; }
	return *ret;
}

void AndroidApp::push(android_app* app, bave::Event event) { self(app).push_event(event); }

void AndroidApp::setup_event_callbacks() {
	m_app.onAppCmd = [](Ptr<android_app> app, int32_t cmd) {
		switch (cmd) {
		case APP_CMD_START: break;
		case APP_CMD_INIT_WINDOW: self(app).resume_render(); break;
		case APP_CMD_TERM_WINDOW: self(app).pause_render(); break;
		case APP_CMD_DESTROY: self(app).destroy(); break;
		case APP_CMD_GAINED_FOCUS: push(app, FocusChange{.in_focus = true}); break;
		case APP_CMD_LOST_FOCUS: push(app, FocusChange{.in_focus = false}); break;
		}
	};

	m_app.onInputEvent = [](Ptr<android_app> app, Ptr<AInputEvent> event) {
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
			auto const key = to_key(AKeyEvent_getKeyCode(event));
			auto const action = to_key_action(AKeyEvent_getAction(event));
			auto const mods = to_key_mods(AKeyEvent_getMetaState(event));
			push(app, KeyInput{.key = key, .action = action, .mods = mods});
			return 1;
		}
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
			auto const position = glm::vec2{AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0)};
			switch (AMotionEvent_getAction(event)) {
			case AMOTION_EVENT_ACTION_MOVE: push(app, CursorMove{.position = position}); break;
			case AMOTION_EVENT_ACTION_DOWN: push(app, TouchTap{.action = Action::ePress, .position = position}); break;
			case AMOTION_EVENT_ACTION_UP: push(app, TouchTap{.action = Action::eRelease, .position = position}); break;
			default: return 0;
			}
			return 1;
		}
		return 0;
	};
}

void AndroidApp::poll_events() {
	auto events = int{};
	auto source = Ptr<android_poll_source>{};
	if (ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) { // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		if (source) { source->process(&m_app, source); }
	}
}

void AndroidApp::tick() {
	if (is_shutting_down() || !m_game) { return; }
	m_game->tick();
}

void AndroidApp::render() {
	if (!m_can_render) { return; }

	auto command_buffer = m_frame_renderer->start_render(m_game->clear_colour);
	if (command_buffer) { m_game->render(command_buffer); }
	m_frame_renderer->finish_render();
}

void AndroidApp::init_graphics() {
	m_render_device = std::make_unique<RenderDevice>(this);
	m_frame_renderer = std::make_unique<FrameRenderer>(m_render_device.get(), &get_data_store());
}

void AndroidApp::pause_render() {
	m_log.debug("render paused");
	m_can_render = false;
}

void AndroidApp::resume_render() {
	if (!m_render_device) {
		start();
	} else {
		m_render_device->recreate_surface();
	}

	m_log.debug("render resumed");
	m_can_render = true;
}

void AndroidApp::start() {
	set_data_store(std::make_unique<AndroidDataStore>(&m_app));
	init_graphics();
	m_game = make_game();
	m_can_render = true;
	m_log.debug("start");
}

void AndroidApp::destroy() {
	m_render_device->get_device().waitIdle();
	m_game.reset();
	m_frame_renderer.reset();
	m_render_device.reset();
	m_can_render = false;
	m_log.debug("destroy");
}
} // namespace bave
