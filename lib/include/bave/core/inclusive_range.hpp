#pragma once
#include <glm/common.hpp>

namespace bave {
template <typename Type>
struct InclusiveRange {
	Type lo{};
	Type hi{};

	constexpr auto clamp(Type const& in) const -> Type {
		using glm::clamp;
		return clamp(in, lo, hi);
	}

	template <typename T>
	constexpr operator InclusiveRange<T>() const {
		return {.lo = static_cast<T>(lo), .hi = static_cast<T>(hi)};
	}
};
} // namespace bave
