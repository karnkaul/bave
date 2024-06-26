#pragma once
#include <bave/app.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/input/event_sink.hpp>
#include <bave/logger.hpp>
#include <vulkan/vulkan.hpp>

namespace bave {
/// \brief Customization point for user code.
///
/// App will create and own a (user-provided sub-class of) Driver.
/// By the time the Driver is created, the App's window and devices are ready to use.
class Driver : public IEventSink {
  public:
	/// \brief Constructor.
	/// \param app Current App instance.
	explicit Driver(App& app) : m_app(app) {}

	[[nodiscard]] auto get_app() const -> App& { return m_app; }

	/// \brief Called every frame.
	///
	/// Update game state here.
	virtual void tick() {}
	/// \brief Called every frame.
	///
	/// Render game state here.
	virtual void render() const {}

	/// \brief Called when window close requested (desktop only).
	/// \returns false to initiate shutdown, true to ignore close request.
	virtual auto prevent_close() -> bool { return false; }

	void on_focus(FocusChange const& /*focus_change*/) override {}
	void on_resize(WindowResize const& /*window_resize*/) override {}
	void on_resize(FramebufferResize const& /*framebuffer_resize*/) override {}
	void on_key(KeyInput const& /*key_input*/) override {}
	void on_char(CharInput const& /*char_input*/) override {}
	void on_tap(PointerTap const& /*pointer_tap*/) override {}
	void on_move(PointerMove const& /*pointer_move*/) override {}
	void on_scroll(MouseScroll const& /*mouse_scroll*/) override {}
	void on_drop(std::span<std::string const> /*paths*/) override {}

	/// \brief Background colour for the next render pass.
	Rgba clear_colour{black_v};

  private:
	void handle_events(std::span<Event const> events);

	App& m_app; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	friend class DesktopApp;
	friend class AndroidApp;
};
} // namespace bave
