#pragma once

namespace bave {
/// \brief 2D Index.
struct Index2D {
	int x{};
	int y{};

	/// \brief Obtain the corresponding 1D index.
	/// \param columns Number of columns in 2D grid.
	/// \returns 1D index into flat array.
	[[nodiscard]] constexpr auto flatten(int const columns) const -> int { return y * columns + x; }
};
} // namespace bave
