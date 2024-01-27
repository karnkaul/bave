#pragma once
#include <bave/app.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/logger.hpp>
#include <vulkan/vulkan.hpp>

namespace bave {
/// \brief Customization point for user code.
///
/// App will create and own a (user-provided sub-class of) Driver.
/// By the time the Driver is created, the App's window and devices are ready to use.
class Driver : public PolyPinned {
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

	virtual void on_focus(FocusChange const& /*focus_change*/) {}
	virtual void on_resize(WindowResize const& /*window_resize*/) {}
	virtual void on_resize(FramebufferResize const& /*framebuffer_resize*/) {}
	virtual void on_key(KeyInput const& /*key_input*/) {}
	virtual void on_char(CharInput const& /*char_input*/) {}
	virtual void on_tap(PointerTap const& /*pointer_tap*/) {}
	virtual void on_move(PointerMove const& /*pointer_move*/) {}
	virtual void on_scroll(MouseScroll const& /*mouse_scroll*/) {}

	virtual void on_drop(std::span<std::string const> /*paths*/) {}

	/// \brief Background colour for the next render pass.
	Rgba clear_colour{black_v};

  private:
	void handle_events(std::span<Event const> events);

	App& m_app; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	friend class DesktopApp;
	friend class AndroidApp;
};
} // namespace bave
