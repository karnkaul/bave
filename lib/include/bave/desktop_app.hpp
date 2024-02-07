#pragma once
#include <bave/app.hpp>
#include <bave/core/ptr.hpp>
#include <bave/detail/dear_imgui.hpp>
#include <bave/driver.hpp>
#include <bave/platform.hpp>
#include <functional>
#include <span>

static_assert(bave::platform_v == bave::Platform::eDesktop);

struct GLFWwindow;

namespace bave {
[[nodiscard]] constexpr auto make_args(int argc, char const* const* argv) -> std::span<char const* const> { return {argv, static_cast<size_t>(argc)}; }

struct Windowed {
	glm::ivec2 extent{1280, 720};
	bool lock_aspect_ratio{true};
	bool decoration{true};
};

struct BorderlessFullscreen {};

/// \brief Variant of possible display modes.
using DisplayMode = std::variant<Windowed, BorderlessFullscreen>;

/// \brief Concrete App for desktop.
class DesktopApp : public App, public detail::IWsi {
  public:
	/// \brief Data needed during construction.
	struct CreateInfo {
		std::span<char const* const> args{};
		CString title{"BaveApp"};
		DisplayMode mode{Windowed{}};
		std::function<Gpu(std::span<Gpu const>)> select_gpu{};
		std::string_view assets_patterns{"assets"};
		bool validation_layers{debug_v};
	};

	/// \brief Constructor.
	/// \param create_info CreateInfo for this instance.
	explicit DesktopApp(CreateInfo create_info);

  private:
	struct LogFile {
		bool init{};
		auto operator==(LogFile const&) const -> bool = default;

		struct Deleter {
			void operator()(LogFile const& lf) const noexcept;
		};
	};

	struct Glfw {
		bool init{};
		auto operator==(Glfw const&) const -> bool = default;

		struct Deleter {
			void operator()(Glfw glfw) const noexcept;
			void operator()(Ptr<GLFWwindow> window) const noexcept;
		};
	};

	static auto self(Ptr<GLFWwindow> window) -> DesktopApp&;
	static void push(Ptr<GLFWwindow> window, Event event);

	auto setup() -> std::optional<ErrCode> final;
	void poll_events() final;
	void tick() final;
	void render() final;

	void do_shutdown() final;

	[[nodiscard]] auto do_get_native_features() const -> FeatureFlags final;
	[[nodiscard]] auto do_get_window_size() const -> glm::ivec2 final;
	[[nodiscard]] auto do_get_framebuffer_size() const -> glm::ivec2 final;

	[[nodiscard]] auto do_get_render_device() const -> RenderDevice& final;
	[[nodiscard]] auto do_get_renderer() const -> Renderer& final;
	[[nodiscard]] auto do_get_driver() const -> Ptr<Driver> final { return m_driver.get(); }

	[[nodiscard]] auto get_instance_extensions() const -> std::span<char const* const> final;
	[[nodiscard]] auto make_surface(vk::Instance instance) const -> vk::SurfaceKHR final;
	[[nodiscard]] auto get_framebuffer_extent() const -> vk::Extent2D final;
	[[nodiscard]] auto select_gpu(std::span<Gpu const> gpus) const -> Gpu final;

	auto do_set_window_size(glm::ivec2 size) -> bool final;
	auto do_set_title(CString title) -> bool final;
	auto do_set_window_icon(std::span<BitmapView const> bitmaps) -> bool final;

	void init_data_store();
	void make_window();
	void init_graphics();

	void update_gamepads();

	CreateInfo m_create_info{};
	ScopedResource<LogFile, LogFile::Deleter> m_log_file{};
	ScopedResource<Glfw, Glfw::Deleter> m_glfw{};
	std::unique_ptr<GLFWwindow, Glfw::Deleter> m_window{};
	std::unique_ptr<RenderDevice> m_render_device{};
	vk::UniqueSurfaceKHR m_surface{};
	std::unique_ptr<Renderer> m_renderer{};
	std::unique_ptr<detail::DearImGui> m_dear_imgui{};
	std::unique_ptr<Driver> m_driver{};
};
} // namespace bave
