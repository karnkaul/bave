#pragma once
#include <bave/input/event.hpp>
#include <bave/input/pointer.hpp>
#include <optional>
#include <span>

namespace bave {
enum class GestureState { eNone, ePinch, eDrag, eTap };

class GestureRecognizer {
  public:
	using State = GestureState;

	static constexpr auto drag_sensitivity_v{5.0f};

	[[nodiscard]] auto get_state() const -> State;

	[[nodiscard]] auto pinch_delta() const -> std::optional<float>;
	[[nodiscard]] auto drag_position() const -> std::optional<glm::vec2>;
	[[nodiscard]] auto tap_down() const -> std::optional<glm::vec2>;
	[[nodiscard]] auto tap_up() const -> std::optional<glm::vec2>;

	void on_tap(PointerTap tap);
	void on_focus();
	void update(std::span<Pointer const> active);

	float drag_sensitivity{drag_sensitivity_v};

  private:
	struct Pinch {
		float distance{};
		float delta{};
	};
	struct Drag {
		glm::vec2 position{};
	};
	struct Tap {
		glm::vec2 position{};
		Action action{};
	};

	struct Primary {
		Action action{Action::eNone};
		glm::vec2 position{};
		bool is_held{};
	};

	auto check_pinch(std::span<Pointer const> active) -> bool;

	std::variant<std::monostate, Pinch, Drag, Tap> m_state{};
	Primary m_primary{};
};
} // namespace bave
