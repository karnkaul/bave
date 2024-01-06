#pragma once
#include <bave/core/pinned.hpp>
#include <bave/logger.hpp>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

namespace bave::detail {
class DeferQueue : public Pinned {
  public:
	template <typename Type>
	void push(std::shared_ptr<Type> obj) {
		if (!obj) { return; }
		auto lock = std::scoped_lock{m_mutex};
		m_current.push_back(std::move(obj));
	}

	void next_frame();
	void clear();

  private:
	using Frame = std::vector<std::shared_ptr<void>>;

	Logger m_log{"Defer"};
	Frame m_current{};
	std::deque<Frame> m_deferred{};
	std::mutex m_mutex{};
};
} // namespace bave::detail
