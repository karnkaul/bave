#pragma once
#include <bave/core/c_string.hpp>
#include <bave/input/action.hpp>
#include <array>
#include <bitset>

namespace bave {
/// \brief Set of gamepad IDs.
///
/// IDs are monotonically enumerated and retained per gamepad, even after disconnection.
/// Thus it is quite possible for e0 to be disconnected while e1 is connected. (The device at e1 will not move to e0).
enum class GamepadId : int { e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, eCOUNT_ };

/// \brief Set of gamepad buttons.
enum class GamepadButton : int {
	eA = 0,
	eB = 1,
	eX = 2,
	eY = 3,

	eCross = eA,
	eCircle = eB,
	eSquare = eX,
	eTriangle = eY,

	eLeftBumper = 4,
	eRightBumper = 5,

	eBack = 6,
	eStart = 7,
	eGuide = 8,

	eLeftThumb = 9,
	eRightThumb = 10,

	eDpadUp = 11,
	eDpadRight = 12,
	eDpadDown = 13,
	eDpadLeft = 14,

	eCOUNT_,
};

/// \brief Set of gamepad axes.
enum class GamepadAxis : int {
	eLeftX = 0,
	eLeftY = 1,
	eRightX = 2,
	eRightY = 3,

	eLeftTrigger = 4,
	eRightTrigger = 5,

	eCOUNT_,
};

/// \brief Read-only state of a gamepad.
struct Gamepad {
	using Id = GamepadId;
	using Button = GamepadButton;
	using Axis = GamepadAxis;

	static constexpr auto buttons_v = static_cast<std::size_t>(Button::eCOUNT_);
	static constexpr auto axes_v = static_cast<std::size_t>(Axis::eCOUNT_);

	/// \brief ID of gamepad.
	Id id{};
	/// \brief Button states.
	std::bitset<buttons_v> button_states{};
	/// \brief Axis values.
	std::array<float, axes_v> axes{};
	/// \brief Reported name of gamepad.
	CString name{};
	/// \brief Whether the gamepad is connected.
	bool connected{};

	/// \brief Check if a button is pressed.
	/// \param button Button to test for.
	/// \returns true if pressed.
	[[nodiscard]] auto is_pressed(Button const button) const -> bool { return button_states.test(static_cast<std::size_t>(button)); }

	/// \brief Obtain an axis value.
	/// \param axis Axis to obtain value of.
	/// \returns Normalized value of axis.
	///
	/// eLeftTrigger and eRightTrigger have a range of [0, 1].
	/// All other axes have a range of [-1, 1].
	[[nodiscard]] auto get_axis(Axis const axis) const -> float { return axes.at(static_cast<std::size_t>(axis)); }
};
} // namespace bave
