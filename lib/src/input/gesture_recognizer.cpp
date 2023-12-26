#include <bave/core/visitor.hpp>
#include <bave/input/gesture_recognizer.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <cassert>

namespace bave {
auto GestureRecognizer::get_state() const -> State {
	auto const visitor = Visitor{
		[](Pinch const&) { return State::ePinch; },
		[](Drag const&) { return State::eDrag; },
		[](Tap const&) { return State::eTap; },
		[](std::monostate) { return State::eNone; },
	};
	return std::visit(visitor, m_state);
}

auto GestureRecognizer::pinch_delta() const -> std::optional<float> {
	if (auto const* pinch = std::get_if<Pinch>(&m_state)) { return pinch->delta; }
	return {};
}

auto GestureRecognizer::drag_position() const -> std::optional<glm::vec2> {
	if (auto const* drag = std::get_if<Drag>(&m_state)) { return drag->position; }
	return {};
}

auto GestureRecognizer::tap_down() const -> std::optional<glm::vec2> {
	auto const* tap = std::get_if<Tap>(&m_state);
	if (tap == nullptr || tap->action != Action::ePress) { return {}; }
	return tap->position;
}

auto GestureRecognizer::tap_up() const -> std::optional<glm::vec2> {
	auto const* tap = std::get_if<Tap>(&m_state);
	if (tap == nullptr || tap->action != Action::eRelease) { return {}; }
	return tap->position;
}

void GestureRecognizer::on_tap(PointerTap const tap) {
	if (tap.pointer.id == PointerId::ePrimary) {
		m_primary = Primary{.action = tap.action, .position = tap.pointer.position, .is_held = tap.action == Action::ePress};
	}
}

void GestureRecognizer::on_focus() {
	m_state = {};
	m_primary = {};
}

void GestureRecognizer::update(std::span<Pointer const> active) {
	auto const primary_action = m_primary.action;
	m_primary.action = {};

	if (check_pinch(active)) { return; }

	if (std::holds_alternative<Tap>(m_state)) { m_state = std::monostate{}; }

	if (primary_action == Action::ePress) {
		m_state = Tap{.position = m_primary.position, .action = Action::ePress};
		return;
	}

	if (primary_action == Action::eRelease && !std::holds_alternative<Drag>(m_state)) {
		m_state = Tap{.position = m_primary.position, .action = Action::eRelease};
		return;
	}

	if (m_primary.is_held) {
		auto const primary = std::find_if(active.begin(), active.end(), [](Pointer const& p) { return p.id == PointerId::ePrimary; });
		assert(primary != active.end());
		if (glm::length(primary->position - m_primary.position) > drag_sensitivity) { m_state = Drag{.position = primary->position}; }
	} else if (std::holds_alternative<Drag>(m_state)) {
		m_state = std::monostate{};
	}

	if (active.empty()) {
		m_state = std::monostate{};
		return;
	}
}

auto GestureRecognizer::check_pinch(std::span<Pointer const> active) -> bool {
	if (active.size() == 2) {
		auto const distance = glm::length(active[1].position - active[0].position);
		if (!std::holds_alternative<Pinch>(m_state)) { m_state = Pinch{.distance = distance}; }
		auto& pinch = std::get<Pinch>(m_state);
		pinch.delta = distance - pinch.distance;
		pinch.distance = distance;
	}

	auto const ret = std::holds_alternative<Pinch>(m_state);
	if (active.empty() && ret) { m_state = std::monostate{}; }
	return ret;
}
} // namespace bave
