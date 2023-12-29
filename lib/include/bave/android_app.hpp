#pragma once
#include <bave/app.hpp>
#include <bave/core/ptr.hpp>
#include <bave/game.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eAndroid);

extern "C" {
struct android_app;
class ANativeWindow;
class AInputEvent;
}

namespace bave {
class AndroidApp : public App, public detail::IWsi {
  public:
	explicit AndroidApp(android_app& app);

  private:
	static auto self(Ptr<android_app> app) -> AndroidApp&;
	static void push(Ptr<android_app> window, Event event);

	auto do_run() -> ErrCode final;
	void do_shutdown() final;

	[[nodiscard]] auto do_get_framebuffer_size() const -> glm::ivec2 final;

	[[nodiscard]] auto do_get_render_device() const -> RenderDevice& final;
	[[nodiscard]] auto do_get_renderer() const -> Renderer& final;

	[[nodiscard]] auto get_instance_extensions() const -> std::span<char const* const> final;
	[[nodiscard]] auto make_surface(vk::Instance instance) const -> vk::SurfaceKHR final;
	[[nodiscard]] auto get_framebuffer_extent() const -> vk::Extent2D final;

	void setup_event_callbacks();
	void poll_events();
	void tick();
	void render();

	void start();
	void destroy();

	void init_graphics();

	void pause_render();
	void resume_render();

	auto handle_motion(Ptr<AInputEvent const> event) -> int;
	auto get_pointer(Ptr<AInputEvent const> event, std::uint32_t index) const -> Pointer;

	void handle_focus(bool gained);

	android_app& m_app; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
	std::unique_ptr<RenderDevice> m_render_device{};
	vk::UniqueSurfaceKHR m_surface{};
	std::unique_ptr<Renderer> m_renderer{};
	std::unique_ptr<Game> m_game{};
	std::optional<AudioStreamer::Pause> m_stream_pause{};
	bool m_can_render{};
};
} // namespace bave
