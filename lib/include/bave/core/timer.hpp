#pragma once
#include <bave/core/time.hpp>
#include <functional>
#include <variant>
#include <vector>

namespace bave {
/// \brief Timer that schedules callbacks.
///
/// Important: pointer/reference captures within callbacks must remain valid until the callback has been invoked.
class Timer {
  public:
	/// \brief Schedule a callback after N ticks.
	/// \param ticks Ticks/frames to wait.
	/// \param callback Callback to invoke.
	void schedule_after(int ticks, std::function<void()> callback);
	/// \brief Schedule a callback after N seconds.
	/// \param delay Seconds to wait.
	/// \param callback Callback to invoke.
	void schedule_after(Seconds delay, std::function<void()> callback);

	/// \brief Update timer state.
	/// \param dt Delta time since last call.
	void tick(Seconds dt);

  private:
	struct Entry {
		std::function<void()> callback{};
		std::variant<Seconds, int> remain{};
	};

	std::vector<Entry> m_entries{};
	std::vector<std::function<void()>> m_to_invoke{};
};
} // namespace bave
