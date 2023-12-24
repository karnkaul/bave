#include <bave/core/error.hpp>
#include <bave/core/visitor.hpp>
#include <bave/game.hpp>

namespace bave {
void Game::handle_events(std::span<Event const> events) {
	auto const visitor = Visitor{
		[this](FocusChange const t) { on_focus(t); }, [this](WindowResize const t) { on_resize(t); }, [this](FramebufferResize const t) { on_resize(t); },
		[this](KeyInput const t) { on_key(t); },	   [this](CharInput const t) { on_char(t); },	   [this](PointerTap const t) { on_tap(t); },
		[this](PointerMove const t) { on_move(t); },  [this](MouseScroll const t) { on_scroll(t); },
	};
	for (auto const event : events) { std::visit(visitor, event); }
}
} // namespace bave
