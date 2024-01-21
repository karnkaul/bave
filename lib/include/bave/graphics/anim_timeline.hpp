#pragma once
#include <bave/core/time.hpp>
#include <span>
#include <string>
#include <vector>

namespace bave {
struct AnimKeyframe {
	std::string tile{};
};

struct AnimTimeline {
	using Keyframe = AnimKeyframe;

	struct View {
		std::span<Keyframe const> keyframes{};
		Seconds duration{1s};

		[[nodiscard]] auto get_keyframe_at(Seconds timestamp) const -> Keyframe const&;
	};

	std::vector<Keyframe> keyframes{};
	Seconds duration{1s};

	[[nodiscard]] auto view() const -> View { return View{.keyframes = keyframes, .duration = duration}; }
};
} // namespace bave
