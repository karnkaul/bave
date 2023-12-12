#pragma once
#include <bave/app.hpp>
#include <bave/core/ptr.hpp>
#include <bave/dear_imgui.hpp>
#include <bave/game.hpp>
#include <bave/graphics/wsi.hpp>
#include <bave/platform.hpp>
#include <functional>
#include <span>

static_assert(bave::platform_v == bave::Platform::eDesktop);

struct GLFWwindow;

namespace bave {
[[nodiscard]] constexpr auto make_args(int argc, char const* const* argv) -> std::span<char const* const> { return {argv, static_cast<size_t>(argc)}; }

class DesktopApp : public App, public IWsi {
  public:
	struct CreateInfo {
		std::span<char const* const> args{};
		CString title{"BaveApp"};
		glm::ivec2 extent{1280, 720};
		std::function<Gpu(std::span<Gpu const>)> select_gpu{};
		bool lock_aspect_ratio{true};
		std::string_view assets_pattern{"assets"};
	};

	explicit DesktopApp(CreateInfo create_info);

  private:
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

	void do_run() final;
	void do_shutdown() final;

	[[nodiscard]] auto do_get_window_size() const -> glm::ivec2 final;
	[[nodiscard]] auto do_get_framebuffer_size() const -> glm::ivec2 final;

	[[nodiscard]] auto do_get_render_device() const -> RenderDevice& final;
	[[nodiscard]] auto do_get_renderer() const -> Renderer& final;

	[[nodiscard]] auto get_instance_extensions() const -> std::span<char const* const> final;
	[[nodiscard]] auto make_surface(vk::Instance instance) const -> vk::SurfaceKHR final;
	[[nodiscard]] auto get_framebuffer_extent() const -> vk::Extent2D final;
	[[nodiscard]] auto select_gpu(std::span<Gpu const> gpus) const -> Gpu final;

	void init_data_store();
	void make_window();
	void init_graphics();

	static void poll_events();
	void tick();
	void render();

	CreateInfo m_create_info{};
	ScopedResource<Glfw, Glfw::Deleter> m_glfw{};
	std::unique_ptr<GLFWwindow, Glfw::Deleter> m_window{};
	std::unique_ptr<RenderDevice> m_render_device{};
	vk::UniqueSurfaceKHR m_surface{};
	std::unique_ptr<Renderer> m_renderer{};
	std::unique_ptr<DearImGui> m_dear_imgui{};
	std::unique_ptr<Game> m_game{};
};
} // namespace bave
