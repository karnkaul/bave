#include <bave/graphics/anim_timeline.hpp>
#include <algorithm>

namespace bave {
auto AnimTimeline::get_tile_at(Seconds const timestamp) const -> std::string_view {
	if (tiles.empty()) { return {}; }
	if (duration <= 0s) { return tiles.back(); }
	auto const ratio = std::clamp(timestamp / duration, 0.0f, 1.0f);
	auto const index = static_cast<std::size_t>(ratio * static_cast<float>(tiles.size()));
	if (index >= tiles.size()) { return tiles.back(); }
	return tiles.at(index);
}
} // namespace bave
