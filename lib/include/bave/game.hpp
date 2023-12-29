#pragma once
#include <bave/app.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/logger.hpp>
#include <vulkan/vulkan.hpp>

namespace bave {
class Game : public PolyPinned {
  public:
	explicit Game(App& app) : m_app(app) {}

	[[nodiscard]] auto get_app() const -> App& { return m_app; }

	virtual void tick() {}
	virtual void render() const {}

	virtual void shutdown() {}

	virtual void on_focus(FocusChange const& /*focus_change*/) {}
	virtual void on_resize(WindowResize const& /*window_resize*/) {}
	virtual void on_resize(FramebufferResize const& /*framebuffer_resize*/) {}
	virtual void on_key(KeyInput const& /*key_input*/) {}
	virtual void on_char(CharInput const& /*char_input*/) {}
	virtual void on_tap(PointerTap const& /*pointer_tap*/) {}
	virtual void on_move(PointerMove const& /*pointer_move*/) {}
	virtual void on_scroll(MouseScroll const& /*mouse_scroll*/) {}

	Rgba clear_colour{black_v};

  private:
	void handle_events(std::span<Event const> events);

	App& m_app; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	friend class DesktopApp;
	friend class AndroidApp;
};
} // namespace bave
