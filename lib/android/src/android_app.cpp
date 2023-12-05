#include <android_native_app_glue.h>
#include <jni.h>
#include <bave/android_app.hpp>
#include <bave/core/error.hpp>
#include <bave/core/ptr.hpp>
#include <bave/game.hpp>
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

struct {
	EGLDisplay display{};
	EGLSurface surface{};
	EGLContext context{};
} g_temp;
} // namespace

AndroidApp::AndroidApp(android_app& app) : m_app(app) { m_app.userData = this; }

void AndroidApp::do_run() {
	app_dummy(); // NOLINT

	setup_event_callbacks();

	auto delta_time = DeltaTime{};
	while (!m_app.destroyRequested) {
		m_dt = delta_time.update();
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

auto AndroidApp::self(Ptr<android_app> app) -> AndroidApp& {
	auto* ret = static_cast<AndroidApp*>(app->userData);
	if (ret == nullptr) { throw Error{"Dereferencing null GLFW Window User Pointer"}; }
	return *ret;
}

void AndroidApp::push(android_app* app, bave::Event event) { self(app).m_events.push_back(event); }

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
	m_events.clear();
	if (ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) { // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		if (source) { source->process(&m_app, source); }
	}
}

void AndroidApp::tick() {
	if (is_shutting_down()) { return; }
	assert(m_game != nullptr);
	m_game->tick();
}

void AndroidApp::render() {
	if (!m_can_render) { return; }

	// temp
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(g_temp.display, g_temp.surface);
}

void AndroidApp::init_graphics() {
	g_temp = {};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config{};

	g_temp.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(g_temp.display, nullptr, nullptr);
	EGLint const attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
	eglChooseConfig(g_temp.display, attribs, nullptr, 0, &numConfigs);
	std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
	assert(supportedConfigs);
	eglChooseConfig(g_temp.display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
	auto i = 0;
	for (; i < numConfigs; i++) {
		auto& cfg = supportedConfigs[i];
		EGLint r, g, b, d;
		if (eglGetConfigAttrib(g_temp.display, cfg, EGL_RED_SIZE, &r) && eglGetConfigAttrib(g_temp.display, cfg, EGL_GREEN_SIZE, &g) &&
			eglGetConfigAttrib(g_temp.display, cfg, EGL_BLUE_SIZE, &b) && eglGetConfigAttrib(g_temp.display, cfg, EGL_DEPTH_SIZE, &d) && r == 8 && g == 8 &&
			b == 8 && d == 0) {
			config = supportedConfigs[i];
			break;
		}
	}
	if (i == numConfigs) { config = supportedConfigs[0]; }

	if (config == nullptr) { throw Error("Unable to initialize EGLConfig"); }

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(g_temp.display, config, EGL_NATIVE_VISUAL_ID, &format);
	g_temp.surface = eglCreateWindowSurface(g_temp.display, config, m_app.window, nullptr);
	g_temp.context = eglCreateContext(g_temp.display, config, nullptr, nullptr);

	if (eglMakeCurrent(g_temp.display, g_temp.surface, g_temp.surface, g_temp.context) == EGL_FALSE) { throw Error("Unable to eglMakeCurrent"); }
}

void AndroidApp::pause_render() {
	m_log.debug("render paused");
	m_can_render = false;
}

void AndroidApp::resume_render() {
	// TODO: recreate swapchain
	if (g_temp.display == EGL_NO_DISPLAY) { init_graphics(); }
	m_log.debug("render resumed");
	m_can_render = true;
}

void AndroidApp::destroy() {
	// temp
	if (g_temp.display != EGL_NO_DISPLAY) {
		eglMakeCurrent(g_temp.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (g_temp.context != EGL_NO_CONTEXT) { eglDestroyContext(g_temp.display, g_temp.context); }
		if (g_temp.surface != EGL_NO_SURFACE) { eglDestroySurface(g_temp.display, g_temp.surface); }
		eglTerminate(g_temp.display);
	}
	g_temp = {};
}
} // namespace bave
