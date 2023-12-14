#pragma once

namespace bave {
struct Index2D {
	int x{};
	int y{};

	[[nodiscard]] constexpr auto flatten(int const columns) const -> int { return y * columns + x; }
};
} // namespace bave
