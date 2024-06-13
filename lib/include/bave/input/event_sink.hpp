#pragma once
#include <bave/core/polymorphic.hpp>
#include <bave/input/event.hpp>

namespace bave {
/// \brief Abstract base class for Event consumers.
class IEventSink : public Polymorphic {
  public:
	virtual void on_focus(FocusChange const& focus_change) = 0;
	virtual void on_resize(WindowResize const& window_resize) = 0;
	virtual void on_resize(FramebufferResize const& framebuffer_resize) = 0;
	virtual void on_key(KeyInput const& key_input) = 0;
	virtual void on_char(CharInput const& char_input) = 0;
	virtual void on_tap(PointerTap const& pointer_tap) = 0;
	virtual void on_move(PointerMove const& pointer_move) = 0;
	virtual void on_scroll(MouseScroll const& mouse_scroll) = 0;
	virtual void on_drop(std::span<std::string const> paths) = 0;
};

/// \brief Concrete base class for Event consumers.
class EventSink : public IEventSink {
  public:
	void on_focus(FocusChange const& /*focus_change*/) override {}
	void on_resize(WindowResize const& /*window_resize*/) override {}
	void on_resize(FramebufferResize const& /*framebuffer_resize*/) override {}
	void on_key(KeyInput const& /*key_input*/) override {}
	void on_char(CharInput const& /*char_input*/) override {}
	void on_tap(PointerTap const& /*pointer_tap*/) override {}
	void on_move(PointerMove const& /*pointer_move*/) override {}
	void on_scroll(MouseScroll const& /*mouse_scroll*/) override {}
	void on_drop(std::span<std::string const> /*paths*/) override {}
};
} // namespace bave
