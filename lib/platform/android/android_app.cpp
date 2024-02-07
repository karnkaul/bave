#include <android/log.h>
#include <android/window.h>
#include <android_native_app_glue.h>
#include <jni.h>
#include <bave/android_app.hpp>
#include <bave/core/error.hpp>
#include <bave/core/ptr.hpp>
#include <bave/driver.hpp>
#include <platform/android/android_data_store.hpp>
#include <cassert>
#include <unordered_map>

namespace bave {
void log::internal::log_message(char level, CString tag, CString message) {
	auto lvl = ANDROID_LOG_INFO;
	switch (level) {
	case 'E': lvl = ANDROID_LOG_ERROR; break;
	case 'W': lvl = ANDROID_LOG_WARN; break;
	case 'D': lvl = ANDROID_LOG_DEBUG; break;
	default: break;
	}

	auto logcat_out = format_thread(message);
	logcat_out.pop_back(); // remove newline

	__android_log_print(lvl, tag.c_str(), "%s", logcat_out.c_str());
}

namespace {
auto to_key(int const key) -> Key {
	static auto const key_map = std::unordered_map<int, Key>{
		{AKEYCODE_SPACE, Key::eSpace},
		{AKEYCODE_APOSTROPHE, Key::eApostrophe},
		{AKEYCODE_COMMA, Key::eComma},
		{AKEYCODE_MINUS, Key::eMinus},
		{AKEYCODE_PERIOD, Key::eFullstop},
		{AKEYCODE_SLASH, Key::eSlash},
		{AKEYCODE_0, Key::e0},
		{AKEYCODE_1, Key::e1},
		{AKEYCODE_2, Key::e2},
		{AKEYCODE_3, Key::e3},
		{AKEYCODE_4, Key::e4},
		{AKEYCODE_5, Key::e5},
		{AKEYCODE_6, Key::e6},
		{AKEYCODE_7, Key::e7},
		{AKEYCODE_8, Key::e8},
		{AKEYCODE_9, Key::e9},
		{AKEYCODE_SEMICOLON, Key::eSemicolon},
		{AKEYCODE_EQUALS, Key::eEqual},
		{AKEYCODE_A, Key::eA},
		{AKEYCODE_B, Key::eB},
		{AKEYCODE_C, Key::eC},
		{AKEYCODE_D, Key::eD},
		{AKEYCODE_E, Key::eE},
		{AKEYCODE_F, Key::eF},
		{AKEYCODE_G, Key::eG},
		{AKEYCODE_H, Key::eH},
		{AKEYCODE_I, Key::eI},
		{AKEYCODE_J, Key::eJ},
		{AKEYCODE_K, Key::eK},
		{AKEYCODE_L, Key::eL},
		{AKEYCODE_M, Key::eM},
		{AKEYCODE_N, Key::eN},
		{AKEYCODE_O, Key::eO},
		{AKEYCODE_P, Key::eP},
		{AKEYCODE_Q, Key::eQ},
		{AKEYCODE_R, Key::eR},
		{AKEYCODE_S, Key::eS},
		{AKEYCODE_T, Key::eT},
		{AKEYCODE_U, Key::eU},
		{AKEYCODE_V, Key::eV},
		{AKEYCODE_W, Key::eW},
		{AKEYCODE_X, Key::eX},
		{AKEYCODE_Y, Key::eY},
		{AKEYCODE_Z, Key::eZ},
		{AKEYCODE_LEFT_BRACKET, Key::eLeftBracket},
		{AKEYCODE_BACKSLASH, Key::eBackslash},
		{AKEYCODE_RIGHT_BRACKET, Key::eRightBracket},
		{AKEYCODE_GRAVE, Key::eGrave},
		{AKEYCODE_BACK, Key::eEscape},
		{AKEYCODE_ESCAPE, Key::eEscape},
		{AKEYCODE_ENTER, Key::eEnter},
		{AKEYCODE_TAB, Key::eTab},
		{AKEYCODE_DEL, Key::eBackspace},
		{AKEYCODE_INSERT, Key::eInsert},
		{AKEYCODE_FORWARD_DEL, Key::eDelete},
		{AKEYCODE_DPAD_RIGHT, Key::eRight},
		{AKEYCODE_DPAD_LEFT, Key::eLeft},
		{AKEYCODE_DPAD_DOWN, Key::eDown},
		{AKEYCODE_DPAD_UP, Key::eUp},
		{AKEYCODE_PAGE_UP, Key::ePageUp},
		{AKEYCODE_PAGE_DOWN, Key::ePageDown},
		{AKEYCODE_HOME, Key::eHome},
		{AKEYCODE_CAPS_LOCK, Key::eCapsLock},
		{AKEYCODE_SCROLL_LOCK, Key::eScrollLock},
		{AKEYCODE_NUM_LOCK, Key::eNumLock},
		{AKEYCODE_F1, Key::eF1},
		{AKEYCODE_F2, Key::eF2},
		{AKEYCODE_F3, Key::eF3},
		{AKEYCODE_F4, Key::eF4},
		{AKEYCODE_F5, Key::eF5},
		{AKEYCODE_F6, Key::eF6},
		{AKEYCODE_F7, Key::eF7},
		{AKEYCODE_F8, Key::eF8},
		{AKEYCODE_F9, Key::eF9},
		{AKEYCODE_F10, Key::eF10},
		{AKEYCODE_F11, Key::eF11},
		{AKEYCODE_F12, Key::eF12},
		{AKEYCODE_SHIFT_LEFT, Key::eLeftShift},
		{AKEYCODE_CTRL_LEFT, Key::eLeftControl},
		{AKEYCODE_ALT_LEFT, Key::eLeftAlt},
		{AKEYCODE_SHIFT_RIGHT, Key::eRightShift},
		{AKEYCODE_CTRL_RIGHT, Key::eRightControl},
		{AKEYCODE_ALT_RIGHT, Key::eRightAlt},
		{AKEYCODE_MENU, Key::eMenu},
	};

	if (auto const it = key_map.find(key); it != key_map.end()) { return it->second; }
	return Key::eUnknown;
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
	if ((mods & AMETA_CTRL_ON) != 0) { ret.set(Mod::eCtrl); }
	if ((mods & AMETA_SHIFT_ON) != 0) { ret.set(Mod::eShift); }
	if ((mods & AMETA_ALT_ON) != 0) { ret.set(Mod::eAlt); }
	if ((mods & AMETA_CAPS_LOCK_ON) != 0) { ret.set(Mod::eCapsLock); }
	if ((mods & AMETA_NUM_LOCK_ON) != 0) { ret.set(Mod::eNumLock); }
	return ret;
}

auto find_index(Ptr<AInputEvent const> event, Pointer::Id id) -> std::optional<std::uint32_t> {
	auto const count = AMotionEvent_getPointerCount(event);
	for (std::uint32_t index = 0; index < count; ++index) {
		if (static_cast<std::int32_t>(id) == AMotionEvent_getPointerId(event, index)) { return index; }
	}
	return {};
}
} // namespace

AndroidApp::AndroidApp(android_app& app, bool validation_layers) : m_app(app), m_validation_layers(validation_layers) { m_app.userData = this; }

auto AndroidApp::setup() -> std::optional<ErrCode> {
	app_dummy();

	setup_event_callbacks();
	return {};
}

void AndroidApp::poll_events() {
	auto events = int{};
	auto source = Ptr<android_poll_source>{};
	if (ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) { // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		if (source) { source->process(&m_app, source); }
	}
	if (m_driver) { m_driver->handle_events(get_events()); }
}

void AndroidApp::tick() {
	if (is_shutting_down() || !m_driver) { return; }
	m_driver->tick();
}

void AndroidApp::render() {
	if (!m_can_render) { return; }

	if (m_renderer->start_render(m_driver->clear_colour)) { m_driver->render(); }
	m_renderer->finish_render();
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

auto AndroidApp::do_get_renderer() const -> Renderer& {
	if (!m_renderer) { throw Error{"Dereferencing null FrameRenderer"}; }
	return *m_renderer;
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

void AndroidApp::push(Ptr<android_app> app, bave::Event event) { self(app).push_event(event); }

void AndroidApp::setup_event_callbacks() {
	m_app.onAppCmd = [](Ptr<android_app> app, int32_t cmd) {
		switch (cmd) {
		case APP_CMD_START: break;
		case APP_CMD_INIT_WINDOW: self(app).resume_render(); break;
		case APP_CMD_TERM_WINDOW: self(app).pause_render(); break;
		case APP_CMD_DESTROY: self(app).destroy(); break;
		case APP_CMD_GAINED_FOCUS: self(app).handle_focus(true); break;
		case APP_CMD_LOST_FOCUS: self(app).handle_focus(false); break;
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
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) { return self(app).handle_motion(event); }
		return 0;
	};
}

void AndroidApp::init_graphics() {
	m_render_device = std::make_unique<RenderDevice>(this, RenderDevice::CreateInfo{.validation_layers = m_validation_layers});
	m_renderer = std::make_unique<Renderer>(m_render_device.get(), &get_data_store());
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
	ANativeActivity_setWindowFlags(m_app.activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
	set_data_store(std::make_unique<AndroidDataStore>(&m_app));
	init_graphics();
	m_driver = boot_driver();
	m_can_render = true;
	m_log.debug("start");
}

void AndroidApp::destroy() {
	get_render_device().get_device().waitIdle();
	m_driver.reset();
	m_renderer.reset();
	m_render_device.reset();
	m_can_render = false;
	m_log.debug("destroy");
}

auto AndroidApp::handle_motion(Ptr<AInputEvent const> event) -> int {
	auto const action = AMotionEvent_getAction(event);
	auto const flags = action & AMOTION_EVENT_ACTION_MASK;
	switch (flags) {
	case AMOTION_EVENT_ACTION_DOWN: {
		auto const pointer = get_pointer(event, 0);
		m_active_pointers.push_back(pointer);
		push_event(PointerTap{.pointer = pointer, .action = Action::ePress});
		break;
	}
	case AMOTION_EVENT_ACTION_UP: {
		m_active_pointers.clear(); // last finger has been lifted
		push_event(PointerTap{.pointer = get_pointer(event, 0), .action = Action::eRelease});
		break;
	}
	case AMOTION_EVENT_ACTION_POINTER_DOWN: {
		auto const index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		auto const pointer = get_pointer(event, static_cast<std::uint32_t>(index));
		m_active_pointers.push_back(pointer);
		push_event(PointerTap{.pointer = pointer, .action = Action::ePress});
		break;
	}
	case AMOTION_EVENT_ACTION_POINTER_UP: {
		auto const index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		auto const pointer = get_pointer(event, static_cast<std::uint32_t>(index));
		std::erase_if(m_active_pointers, [&](Pointer const& p) { return p.id == pointer.id; });
		push_event(PointerTap{.pointer = pointer, .action = Action::eRelease});
		break;
	}
	case AMOTION_EVENT_ACTION_MOVE: {
		for (auto& pointer : m_active_pointers) {
			if (auto const index = find_index(event, pointer.id)) {
				pointer = get_pointer(event, *index);
				push_event(PointerMove{.pointer = pointer});
			}
		}
		break;
	}
	default: return 0;
	}
	return 1;
}

auto AndroidApp::get_pointer(Ptr<AInputEvent const> event, std::uint32_t const index) const -> Pointer {
	return Pointer{
		.id = static_cast<Pointer::Id>(AMotionEvent_getPointerId(event, index)),
		.position = screen_to_framebuffer({AMotionEvent_getX(event, index), AMotionEvent_getY(event, index)}),
	};
}

void AndroidApp::handle_focus(bool gained) {
	if (gained) {
		if (m_stream_pause) {
			get_audio_streamer().resume(*m_stream_pause);
			m_stream_pause.reset();
		}
		push_event(FocusChange{.in_focus = true});
	} else {
		m_active_pointers.clear();
		m_gesture_recognizer.on_focus();
		m_stream_pause = get_audio_streamer().pause();
		push_event(FocusChange{.in_focus = false});
	}
}
} // namespace bave
