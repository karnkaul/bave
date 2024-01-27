#pragma once
#include <bave/core/time.hpp>
#include <string>
#include <vector>

namespace bave {
/// \brief Animation Timeline as a cycle of atlas tiles.
struct AnimTimeline {
	/// \brief IDS of tiles to cycle through.
	std::vector<std::string> tiles{};
	/// \brief Duration of timeline.
	Seconds duration{1s};

	/// \brief Obtain the tile ID at a given timestamp.
	/// \param timestamp Timestamp to get tile ID for.
	/// \return Tile ID corresponding to timestamp. Empty string if tiles is empty.
	[[nodiscard]] auto get_tile_at(Seconds timestamp) const -> std::string_view;
};
} // namespace bave
