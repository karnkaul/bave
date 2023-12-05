#pragma once
#include <bave/build_version.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/core/time.hpp>
#include <bave/event.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <span>
#include <vector>

namespace bave {
enum struct ErrCode : int { eSuccess = 0, eFailure = 1 };

class App : public PolyPinned {
  public:
	explicit App(std::string tag = "App");

	void set_game(std::unique_ptr<class Game> game);
	auto run() -> ErrCode;

	void shutdown();
	[[nodiscard]] auto is_shutting_down() const { return m_shutting_down; }

	[[nodiscard]] auto get_events() const -> std::span<Event const> { return m_events; }
	[[nodiscard]] auto get_dt() const -> Seconds { return m_dt; }
	[[nodiscard]] auto get_window_size() const -> glm::ivec2 { return do_get_window_size(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }

  protected:
	struct Deleter {
		void operator()(Game* ptr) const noexcept;
	};

	std::unique_ptr<Game, Deleter> m_game{};
	Logger m_log{};

	std::vector<Event> m_events{};
	Seconds m_dt{};

  private:
	virtual void do_run() = 0;
	virtual void do_shutdown() = 0;

	[[nodiscard]] virtual auto do_get_window_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }
	[[nodiscard]] virtual auto do_get_framebuffer_size() const -> glm::ivec2 = 0;

	bool m_shutting_down{};
};
} // namespace bave
