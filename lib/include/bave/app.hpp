#pragma once
#include <imgui.h>
#include <bave/audio/audio_device.hpp>
#include <bave/audio/audio_streamer.hpp>
#include <bave/build_version.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/core/time.hpp>
#include <bave/core/timer.hpp>
#include <bave/data_store.hpp>
#include <bave/graphics/renderer.hpp>
#include <bave/graphics/shader.hpp>
#include <bave/input/event.hpp>
#include <bave/input/gesture_recognizer.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>
#include <capo/capo.hpp>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace bave {
enum struct ErrCode : int { eSuccess = 0, eFailure = 1 };

class App : public PolyPinned {
  public:
	using Bootloader = std::function<std::unique_ptr<class Driver>(App&)>;

	explicit App(std::string tag = "App");

	void set_bootloader(Bootloader bootloader);
	void set_data_store(std::unique_ptr<DataStore> data_store);

	auto run() -> ErrCode;

	void shutdown();
	[[nodiscard]] auto is_shutting_down() const { return m_shutting_down; }

	auto set_window_size(glm::ivec2 size) -> bool { return do_set_window_size(size); }
	auto set_framebuffer_size(glm::ivec2 size) -> bool;

	[[nodiscard]] auto get_data_store() const -> DataStore& { return *m_data_store; }
	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return do_get_render_device(); }
	[[nodiscard]] auto get_renderer() const -> Renderer const& { return do_get_renderer(); }
	[[nodiscard]] auto get_audio_device() const -> AudioDevice& { return *m_audio_device; }
	[[nodiscard]] auto get_audio_streamer() const -> AudioStreamer& { return *m_audio_streamer; }

	[[nodiscard]] auto get_events() const -> std::span<Event const> { return m_events; }
	[[nodiscard]] auto get_file_drops() const -> std::span<std::string const> { return m_drops; }
	[[nodiscard]] auto get_active_pointers() const -> std::span<Pointer const> { return m_active_pointers; }
	[[nodiscard]] auto get_gesture_recognizer() const -> GestureRecognizer const& { return m_gesture_recognizer; }
	[[nodiscard]] auto get_dt() const -> Seconds { return m_dt.dt; }
	[[nodiscard]] auto get_window_size() const -> glm::ivec2 { return do_get_window_size(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }
	[[nodiscard]] auto get_display_ratio() const -> glm::vec2;
	[[nodiscard]] auto get_pipeline_cache() const -> detail::PipelineCache& { return do_get_renderer().get_pipeline_cache(); }

	[[nodiscard]] auto get_timer() -> Timer& { return m_timer; }
	[[nodiscard]] auto get_driver() const -> Ptr<Driver> { return do_get_driver(); }

	[[nodiscard]] auto load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader>;

	auto change_mount_point(std::string_view directory) -> bool;
	auto set_title(CString title) -> bool { return do_set_title(title); }

  protected:
	void start_next_frame();
	void push_event(Event event);
	void push_drop(std::string path);
	[[nodiscard]] auto boot_driver() -> std::unique_ptr<Driver>;

	[[nodiscard]] auto screen_to_framebuffer(glm::vec2 position) const -> glm::vec2;

	Logger m_log{};
	std::vector<Pointer> m_active_pointers{};
	GestureRecognizer m_gesture_recognizer{};

  private:
	virtual auto setup() -> std::optional<ErrCode> = 0;
	virtual void poll_events() = 0;
	virtual void tick() = 0;
	virtual void render() = 0;

	virtual void do_shutdown() = 0;

	[[nodiscard]] virtual auto do_get_window_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }
	[[nodiscard]] virtual auto do_get_framebuffer_size() const -> glm::ivec2 = 0;

	[[nodiscard]] virtual auto do_get_render_device() const -> RenderDevice& = 0;
	[[nodiscard]] virtual auto do_get_renderer() const -> Renderer& = 0;
	[[nodiscard]] virtual auto do_get_driver() const -> Ptr<Driver> = 0;

	virtual auto do_set_window_size(glm::ivec2 size) -> bool = 0;
	virtual auto do_set_title(CString /*title*/) -> bool { return false; }

	void pre_tick();

	std::function<std::unique_ptr<Driver>(App&)> m_bootloader{};
	std::unique_ptr<DataStore> m_data_store{std::make_unique<DataStore>()};
	std::unique_ptr<AudioDevice> m_audio_device{};
	std::unique_ptr<AudioStreamer> m_audio_streamer{};

	std::vector<std::string> m_drops{};
	std::vector<Event> m_events{};
	DeltaTime m_dt{};
	Timer m_timer{};

	bool m_shutting_down{};
};
} // namespace bave
