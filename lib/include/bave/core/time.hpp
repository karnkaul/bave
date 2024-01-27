#pragma once
#include <chrono>

using namespace std::chrono_literals;

namespace bave {
/// \brief Number of seconds as float.
using Seconds = std::chrono::duration<float>;
/// \brief Clock used for engine time.
using Clock = std::chrono::steady_clock;

/// \brief Wrapper to compute delta time.
struct DeltaTime {
	/// \brief Starting time point.
	Clock::time_point frame_start{Clock::now()};
	/// \brief Previous delta time.
	Seconds dt{};

	/// \brief Update delta time.
	/// \returns Updated delta time.
	auto update() -> Seconds {
		auto const now = Clock::now();
		dt = Seconds{now - frame_start};
		frame_start = now;
		return dt;
	}
};
} // namespace bave
