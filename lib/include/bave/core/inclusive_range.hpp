#pragma once
#include <glm/common.hpp>

namespace bave {
template <typename T>
struct InclusiveRange {
	T lo{};
	T hi{};

	constexpr auto clamp(T const& in) const -> T {
		using glm::clamp;
		return clamp(in, lo, hi);
	}

	template <typename Type>
	constexpr operator InclusiveRange<Type>() const {
		return {.lo = static_cast<Type>(lo), .hi = static_cast<Type>(hi)};
	}
};
} // namespace bave
