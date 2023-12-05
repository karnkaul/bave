#pragma once
#include <bave/app.hpp>
#include <bave/core/ptr.hpp>
#include <bave/game.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eAndroid);

extern "C" {
struct android_app;
}

namespace bave {
class AndroidApp : public App { // NOLINT(cppcoreguidelines-special-member-functions)
  public:
	explicit AndroidApp(android_app& app);

  private:
	static auto self(Ptr<android_app> app) -> AndroidApp&;
	static void push(Ptr<android_app> window, Event event);

	void do_run() final;
	void do_shutdown() final;

	[[nodiscard]] auto do_get_framebuffer_size() const -> glm::ivec2 final;

	void setup_event_callbacks();
	void poll_events();
	void tick();
	void render();
	void destroy();

	void init_graphics();

	void pause_render();
	void resume_render();

	android_app& m_app;
	bool m_can_render{};
};
} // namespace bave
