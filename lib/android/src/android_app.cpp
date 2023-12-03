#include <android_native_app_glue.h>
#include <jni.h>
#include <bave/android_app.hpp>
#include <bave/core/error.hpp>
#include <bave/core/ptr.hpp>
#include <bave/game.hpp>

namespace {
auto g_app = std::optional<bave::AndroidApp>{};
auto const g_log = bave::Logger{"AndroidApp"};
} // namespace

void handle_cmd(android_app* app, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_INIT_WINDOW: {
		g_app.emplace(*app);
		break;
	}

	case APP_CMD_TERM_WINDOW: {
		g_app.reset();
		break;
	}
	}
}

namespace bave {
AndroidApp::AndroidApp(android_app& app) : m_app(app) { m_game = std::make_unique<Game>(*this); }

AndroidApp::~AndroidApp() { m_log.log(log::info_v, "exit"); }

void AndroidApp::set_game(std::unique_ptr<Game> game) {
	if (!game) { return; }
	m_game = std::move(game);
}

auto AndroidApp::run() -> ErrCode {
	app_dummy(); // NOLINT

	m_app.onAppCmd = handle_cmd;

	m_log.log(log::info_v, "run");

	auto events = int{};
	auto source = bave::Ptr<android_poll_source>{};
	do {																					// NOLINT
		if (ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) { // NOLINT
			if (source) { source->process(&m_app, source); }
		}
	} while (!m_app.destroyRequested);

	m_log.log(log::info_v, "exit");

	return ErrCode::eSuccess;
}
} // namespace bave
