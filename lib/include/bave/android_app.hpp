#pragma once
#include <bave/app.hpp>
#include <bave/core/ptr.hpp>
#include <bave/driver.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eAndroid);

extern "C" {
struct android_app;
class ANativeWindow;
class AInputEvent;
}

namespace bave {
class AndroidApp : private App, private detail::IWsi {
  public:
	static constexpr auto msaa_v = vk::SampleCountFlagBits{vk::SampleCountFlagBits::e1};

	explicit AndroidApp(android_app& app, vk::SampleCountFlagBits msaa = msaa_v, bool validation_layers = debug_v);

	using App::run;
	using App::set_bootloader;

  private:
	static auto self(Ptr<android_app> app) -> AndroidApp&;
	static void push(Ptr<android_app> window, Event event);

	auto setup() -> std::optional<ErrCode> final;
	void poll_events() final;
	void tick() final;
	void render() final;

	void do_shutdown() final;
	[[nodiscard]] auto get_is_shutting_down() const -> bool final;

	[[nodiscard]] auto do_get_persistent_dir() const -> std::string_view final { return m_persistent_dir; }

	[[nodiscard]] auto do_get_framebuffer_size() const -> glm::ivec2 final;

	[[nodiscard]] auto do_get_render_device() const -> RenderDevice& final;
	[[nodiscard]] auto do_get_renderer() const -> Renderer& final;
	[[nodiscard]] auto do_get_driver() const -> Ptr<Driver> final { return m_driver.get(); }

	[[nodiscard]] auto get_instance_extensions() const -> std::span<char const* const> final;
	[[nodiscard]] auto make_surface(vk::Instance instance) const -> vk::SurfaceKHR final;
	[[nodiscard]] auto get_framebuffer_extent() const -> vk::Extent2D final;

	auto do_set_window_size(glm::ivec2 /*size*/) -> bool final { return false; }
	void do_wait_render_device_idle() final;

	void setup_event_callbacks();

	void start();
	void destroy();

	void init_graphics();

	void pause_render();
	void resume_render();

	auto handle_motion(Ptr<AInputEvent const> event) -> int;
	auto get_pointer(Ptr<AInputEvent const> event, std::uint32_t index) const -> Pointer;

	void handle_focus(bool gained);

	android_app& m_app; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
	vk::SampleCountFlagBits m_msaa;
	bool m_validation_layers;
	std::string m_persistent_dir{};

	std::unique_ptr<RenderDevice> m_render_device{};
	vk::UniqueSurfaceKHR m_surface{};
	std::unique_ptr<Renderer> m_renderer{};
	std::unique_ptr<Driver> m_driver{};
	bool m_can_render{};

	std::optional<AudioStreamer::Pause> m_stream_pause{};
};
} // namespace bave
