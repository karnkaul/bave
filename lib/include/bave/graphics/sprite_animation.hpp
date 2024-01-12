#pragma once
#include <bave/core/time.hpp>
#include <string>

namespace bave {
struct SpriteAnimation {
	std::vector<std::string> tile_ids{};
	Seconds duration{};

	[[nodiscard]] auto get_tile_at(Seconds timestamp) const -> std::string_view;
};
} // namespace bave
