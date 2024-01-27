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
/// \brief Error code.
enum struct ErrCode : int { eSuccess = EXIT_SUCCESS, eFailure = EXIT_FAILURE };

/// \brief Application API and entrypoint.
///
/// Owns window, devices, game and event loops.
/// Customized via Driver sub-class.
class App : public PolyPinned {
  public:
	/// \brief Individual feature flags.
	struct Feature {
		static constexpr std::size_t resizeable{0};
		static constexpr std::size_t has_title{1};
		static constexpr std::size_t has_icon{2};
		static constexpr std::size_t validation_layers{3};

		static constexpr std::size_t count_v{8};
	};
	/// \brief Bitset of feature flags.
	using FeatureFlags = std::bitset<Feature::count_v>;

	/// \brief Driver Factory.
	using Bootloader = std::function<std::unique_ptr<class Driver>(App&)>;

	/// \brief Set Bootloader.
	/// \param bootloader Driver factory callback.
	///
	/// App will create a Driver after successful initialization of the window and devices.
	void set_bootloader(Bootloader bootloader);
	/// \brief Set custom DataStore.
	/// \param data_store store instance to use.
	/// \pre data_store should not be null.
	void set_data_store(std::unique_ptr<DataStore> data_store);

	/// \brief Run the game loop.
	/// \returns Error code.
	auto run() -> ErrCode;

	/// \brief Request shutdown.
	void shutdown();
	/// \brief Check if shutting down.
	/// \returns true if shutting down.
	[[nodiscard]] auto is_shutting_down() const { return m_shutting_down; }

	/// \brief Set window size.
	/// \returns true if success.
	///
	/// Android windows cannot be resized.
	auto set_window_size(glm::ivec2 const size) -> bool { return do_set_window_size(size); }
	/// \brief Set framebuffer size.
	/// \returns true if success.
	///
	/// Android windows cannot be resized.
	auto set_framebuffer_size(glm::ivec2 size) -> bool;
	/// \brief Set window icons.
	/// \returns true if success.
	///
	/// Android windows do not have icons.
	auto set_window_icon(std::span<BitmapView const> bitmaps) -> bool { return do_set_window_icon(bitmaps); }
	/// \brief Set the window title.
	/// \returns true if success.
	///
	/// Android windows do not have a title.
	auto set_title(CString title) -> bool { return do_set_title(title); }

	/// \brief Load a shader for drawing.
	/// \param vertex URI of vertex shader (SPIR-V).
	/// \param fragment URI of fragment shader (SPIR-V).
	/// \returns Shader if loaded successfully.
	///
	/// Loaded shaders are cached and not reloaded on every call.
	/// A Shader instance is intended to be temporary, within a draw scope.
	[[nodiscard]] auto load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader>;

	/// \brief Change the data store mount point.
	/// \param directory Directory to mount.
	/// \returns true on success.
	///
	/// Mount point is fixed on Android.
	auto change_mount_point(std::string_view directory) -> bool;

	[[nodiscard]] auto get_features() const -> FeatureFlags;

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

  protected:
	explicit App(std::string tag = "App");

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

	[[nodiscard]] virtual auto do_get_native_features() const -> FeatureFlags { return {}; }
	[[nodiscard]] virtual auto do_get_window_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }
	[[nodiscard]] virtual auto do_get_framebuffer_size() const -> glm::ivec2 = 0;

	[[nodiscard]] virtual auto do_get_render_device() const -> RenderDevice& = 0;
	[[nodiscard]] virtual auto do_get_renderer() const -> Renderer& = 0;
	[[nodiscard]] virtual auto do_get_driver() const -> Ptr<Driver> = 0;

	virtual auto do_set_window_size(glm::ivec2 size) -> bool = 0;
	virtual auto do_set_title(CString /*title*/) -> bool { return false; }
	virtual auto do_set_window_icon(std::span<BitmapView const> /*bitmaps*/) -> bool { return false; }

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
