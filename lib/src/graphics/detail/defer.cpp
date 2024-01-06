#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/defer.hpp>

namespace bave::detail {
void DeferQueue::next_frame() {
	auto lock = std::scoped_lock{m_mutex};
	auto count = size_t{};
	m_deferred.push_back(std::move(m_current));
	while (m_deferred.size() > buffering_v) {
		count += m_deferred.front().size();
		m_deferred.pop_front();
	}
	if (count > 0) { m_log.debug("{} deferred items destroyed", count); }
}

void DeferQueue::clear() {
	auto lock = std::scoped_lock{m_mutex};
	m_current.clear();
	m_deferred.clear();
}
} // namespace bave::detail
