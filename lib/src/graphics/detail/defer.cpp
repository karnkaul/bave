#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/defer.hpp>
#include <algorithm>

namespace bave::detail {
void DeferQueue::next_frame() {
	auto lock = std::scoped_lock{m_mutex};
	m_queue.front().clear();
	std::rotate(m_queue.begin(), m_queue.begin() + 1, m_queue.end());
}

void DeferQueue::clear() {
	auto lock = std::scoped_lock{m_mutex};
	for (auto& frame : m_queue) { frame.clear(); }
}
} // namespace bave::detail
