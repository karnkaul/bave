#pragma once
#include <bave/input/action.hpp>
#include <bave/input/key.hpp>
#include <bave/input/mods.hpp>
#include <bave/input/pointer.hpp>
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

enum struct MouseButton : int { eLeft = 0, eRight = 1, eMid = 2 };

struct PointerTap {
	Pointer pointer{};
	Action action{};
	KeyMods mods{};
	MouseButton button{MouseButton::eLeft};
};

struct PointerMove {
	Pointer pointer{};
};

struct MouseScroll {
	glm::vec2 delta{};
};

using Event = std::variant<FocusChange, WindowResize, FramebufferResize, KeyInput, CharInput, PointerTap, PointerMove, MouseScroll>;
} // namespace bave
