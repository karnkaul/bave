#pragma once
#include <bave/app.hpp>
#include <bave/core/ptr.hpp>
#include <bave/game.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eDesktop);

struct GLFWwindow;

namespace bave {
[[nodiscard]] constexpr auto make_args(int argc, char const* const* argv) -> std::span<char const* const> { return {argv, static_cast<size_t>(argc)}; }

class DesktopApp : public App {
  public:
	struct CreateInfo {
		std::span<char const* const> args{};
		CString title{"BaveApp"};
		glm::ivec2 extent{1280, 720};
	};

	explicit DesktopApp(CreateInfo const& create_info);

  private:
	static auto self(Ptr<GLFWwindow> window) -> DesktopApp&;
	static void push(Ptr<GLFWwindow> window, Event event);

	void do_run() final;
	void do_shutdown() final;

	[[nodiscard]] auto do_get_window_size() const -> glm::ivec2 final;
	[[nodiscard]] auto do_get_framebuffer_size() const -> glm::ivec2 final;

	void make_window();

	void poll_events();
	void tick();
	void render();

	CreateInfo m_create_info{};
	Ptr<GLFWwindow> m_window{};
};
} // namespace bave
