#include <bave/graphics/sprite_animation.hpp>
#include <algorithm>

namespace bave {
SpriteAnimation::SpriteAnimation(std::vector<KeyFrame> key_frames, Seconds duration) : m_key_frames(std::move(key_frames)), m_duration(duration) {
	sort_keyframes();
}

SpriteAnimation::SpriteAnimation(std::vector<std::string> tile_ids, Seconds duration) : m_duration(duration) {
	auto const avg_duration = Seconds{duration / static_cast<float>(tile_ids.size())};
	auto timestamp = Seconds{};
	for (auto& tile_id : tile_ids) {
		auto key_frame = KeyFrame{.tile_id = std::move(tile_id), .timestamp = timestamp};
		timestamp += avg_duration;
		if (!key_frame.tile_id.empty()) { m_key_frames.push_back(std::move(key_frame)); }
	}
	sort_keyframes();
}

auto SpriteAnimation::get_tile_at(Seconds const timestamp) const -> std::string_view {
	if (m_key_frames.empty()) { return {}; }

	auto const comp = [](KeyFrame const& kf, Seconds const timestamp) { return kf.timestamp <= timestamp; };
	auto next = std::lower_bound(m_key_frames.begin(), m_key_frames.end(), timestamp, comp);
	if (next == m_key_frames.end()) { return m_key_frames.back().tile_id; }
	if (next == m_key_frames.begin()) { return next->tile_id; }
	return (--next)->tile_id;
}

void SpriteAnimation::sort_keyframes() {
	std::sort(m_key_frames.begin(), m_key_frames.end(), [](KeyFrame const& a, KeyFrame const& b) { return a.timestamp < b.timestamp; });
}
} // namespace bave
