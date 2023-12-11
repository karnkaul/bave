#pragma once
#include <bave/build_version.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/core/time.hpp>
#include <bave/data_store.hpp>
#include <bave/event.hpp>
#include <bave/graphics/frame_renderer.hpp>
#include <bave/graphics/shader.hpp>
#include <bave/logger.hpp>
#include <functional>
#include <memory>
#include <span>
#include <vector>

#if defined(BAVE_IMGUI)
#include <imgui.h>
#define IFBAVEIMGUI(expr) expr
#else
#define IFBAVEIMGUI(expr)
#endif

namespace bave {
enum struct ErrCode : int { eSuccess = 0, eFailure = 1 };

class App : public PolyPinned {
  public:
	explicit App(std::string tag = "App");

	void set_game_factory(std::function<std::unique_ptr<class Game>(App&)> factory);
	void set_data_store(std::unique_ptr<DataStore> data_store);

	auto run() -> ErrCode;

	void shutdown();
	[[nodiscard]] auto is_shutting_down() const { return m_shutting_down; }

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return do_get_render_device(); }
	[[nodiscard]] auto get_frame_renderer() const -> FrameRenderer const& { return do_get_frame_renderer(); }
	[[nodiscard]] auto get_data_store() const -> DataStore&;

	[[nodiscard]] auto get_events() const -> std::span<Event const> { return m_events; }
	[[nodiscard]] auto get_dt() const -> Seconds { return m_dt.dt; }
	[[nodiscard]] auto get_window_size() const -> glm::ivec2 { return do_get_window_size(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::ivec2 { return do_get_framebuffer_size(); }
	[[nodiscard]] auto get_pipeline_cache() const -> PipelineCache& { return do_get_frame_renderer().get_pipeline_cache(); }

	[[nodiscard]] auto load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader>;

	RenderView render_view{};

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
	[[nodiscard]] virtual auto do_get_frame_renderer() const -> FrameRenderer& = 0;

	std::function<std::unique_ptr<Game>(App&)> m_game_factory{};
	std::unique_ptr<DataStore> m_data_store{};

	std::vector<Event> m_events{};
	DeltaTime m_dt{};

	bool m_shutting_down{};
};
} // namespace bave
