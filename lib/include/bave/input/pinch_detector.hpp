#pragma once
#include <bave/input/pointer.hpp>
#include <optional>
#include <span>

namespace bave {
class PinchDetector {
  public:
	auto update(std::span<Pointer const> active) -> std::optional<float>;

	[[nodiscard]] auto get_delta() const -> float { return m_delta; }

  private:
	std::optional<float> m_distance{};
	float m_delta{};
};
} // namespace bave
