#pragma once
#include <bave/core/time.hpp>
#include <functional>
#include <variant>
#include <vector>

namespace bave {
class Timer {
  public:
	void schedule_after(int ticks, std::function<void()> callback);
	void schedule_after(Seconds delay, std::function<void()> callback);

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
