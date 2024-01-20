#pragma once
#include <bave/core/time.hpp>
#include <string>
#include <vector>

namespace bave {
struct AnimTimeline {
	std::vector<std::string> tiles{};
	Seconds duration{};

	[[nodiscard]] auto get_tile_at(Seconds timestamp) const -> std::string_view;
};
} // namespace bave
