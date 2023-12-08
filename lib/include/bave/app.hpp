#pragma once
#include <bave/build_version.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/core/time.hpp>
#include <bave/event.hpp>
#include <bave/graphics/frame_renderer.hpp>
#include <bave/logger.hpp>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace bave {
enum struct ErrCode : int { eSuccess = 0, eFailure = 1 };

class App : public PolyPinned {
  public:
	explicit App(std::string tag = "App");

	void set_game_factory(std::function<std::unique_ptr<class Game>(App&)> factory);

	auto run() -> ErrCode;

	void shutdown();
	[[nodiscard]] auto is_shutting_down() const { return m_shutting_down; }

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return do_get_render_device(); }

	[[nodiscard]] auto get_events() const -> std::span<Event const> { return m_events; }
	[[nodiscard]] auto get_dt() const -> Seconds { return m_dt.dt; }
	[[nodiscard]] auto get_window_size() const -> glm::ivec2 { return do_get_window_size(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }

  protected:
	void start_next_frame();
	void push_event(Event event) { m_events.push_back(event); }
	[[nodiscard]] auto make_game() -> std::unique_ptr<Game>;

	Logger m_log{};

  private:
	virtual void do_run() = 0;
	virtual void do_shutdown() = 0;

	[[nodiscard]] virtual auto do_get_window_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }
	[[nodiscard]] virtual auto do_get_framebuffer_size() const -> glm::ivec2 = 0;

	[[nodiscard]] virtual auto do_get_render_device() const -> RenderDevice& = 0;

	std::function<std::unique_ptr<Game>(App&)> m_game_factory{};

	std::vector<Event> m_events{};
	DeltaTime m_dt{};

	bool m_shutting_down{};
};
} // namespace bave
