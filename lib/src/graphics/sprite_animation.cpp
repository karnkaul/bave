#include <bave/graphics/sprite_animation.hpp>
#include <algorithm>

namespace bave {
auto SpriteAnimation::get_tile_at(Seconds const timestamp) const -> std::string_view {
	if (tile_ids.empty()) { return {}; }
	if (duration <= 0s) { return tile_ids.back(); }
	auto const ratio = std::clamp(timestamp / duration, 0.0f, 1.0f);
	auto const index = static_cast<std::size_t>(ratio * static_cast<float>(tile_ids.size()));
	if (index >= tile_ids.size()) { return tile_ids.back(); }
	return tile_ids.at(index);
}
} // namespace bave
