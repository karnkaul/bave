#include <bave/graphics/anim_timeline.hpp>
#include <algorithm>

namespace bave {
auto AnimTimeline::View::get_keyframe_at(Seconds const timestamp) const -> Keyframe const& {
	static auto const s_empty = Keyframe{};
	if (keyframes.empty()) { return s_empty; }
	if (duration <= 0s) { return keyframes.back(); }
	auto const ratio = std::clamp(timestamp / duration, 0.0f, 1.0f);
	auto const index = static_cast<std::size_t>(ratio * static_cast<float>(keyframes.size()));
	if (index >= keyframes.size()) { return keyframes.back(); }
	return keyframes[index];
}
} // namespace bave
