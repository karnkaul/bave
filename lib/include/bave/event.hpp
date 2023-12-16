#pragma once
#include <bave/key_event.hpp>
#include <glm/vec2.hpp>
#include <variant>

namespace bave {
struct FocusChange {
	bool in_focus{};
};

struct WindowResize {
	glm::ivec2 extent{};
};

struct FramebufferResize {
	glm::ivec2 extent{};
};

struct KeyInput {
	Key key{};
	Action action{};
	KeyMods mods{};
	int scancode{};
};

struct CharInput {
	std::uint32_t code{};
};

struct CursorMove {
	int id{};
	glm::vec2 position{};
};

struct MouseScroll {
	glm::vec2 delta{};
};

struct MouseClick {
	int id{};
	Action action{};
	KeyMods mods{};
	glm::vec2 position{};
};
using TouchTap = MouseClick;

using Event = std::variant<FocusChange, WindowResize, FramebufferResize, KeyInput, CharInput, CursorMove, MouseScroll, MouseClick>;
} // namespace bave
