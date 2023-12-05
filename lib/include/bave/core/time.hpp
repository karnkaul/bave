#pragma once
#include <chrono>

using namespace std::chrono_literals;

namespace bave {
using Seconds = std::chrono::duration<float>;
using Clock = std::chrono::steady_clock;

struct DeltaTime {
	Clock::time_point frame_start{Clock::now()};
	Seconds dt{};

	auto update() -> Seconds {
		auto const now = Clock::now();
		dt = Seconds{now - frame_start};
		frame_start = now;
		return dt;
	}
};
} // namespace bave
