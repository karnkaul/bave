#include <bave/core/timer.hpp>
#include <bave/core/visitor.hpp>

namespace bave {
void Timer::schedule_after(int ticks, std::function<void()> callback) { m_entries.push_back(Entry{.callback = std::move(callback), .remain = ticks}); }

void Timer::schedule_after(Seconds delay, std::function<void()> callback) { m_entries.push_back(Entry{.callback = std::move(callback), .remain = delay}); }

void Timer::tick(Seconds const dt) {
	auto const has_expired = Visitor{
		[dt](Seconds& remain) {
			remain -= dt;
			return remain <= 0s;
		},
		[](int& remain) { return --remain <= 0; },
	};
	auto const should_erase = [&](Entry& entry) {
		if (std::visit(has_expired, entry.remain)) {
			m_to_invoke.push_back(std::move(entry.callback));
			return true;
		}
		return false;
	};

	std::erase_if(m_entries, should_erase);

	for (auto const& callback : m_to_invoke) { callback(); }
	m_to_invoke.clear();
}
} // namespace bave
