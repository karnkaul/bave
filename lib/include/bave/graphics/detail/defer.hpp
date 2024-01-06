#pragma once
#include <bave/graphics/detail/buffering.hpp>
#include <array>
#include <memory>
#include <mutex>
#include <vector>

namespace bave::detail {
class DeferQueue {
  public:
	template <typename Type>
	void push(std::shared_ptr<Type> obj) {
		if (!obj) { return; }
		auto lock = std::scoped_lock{m_mutex};
		m_queue.back().push_back(std::move(obj));
	}

	void next_frame();
	void clear();

  private:
	using Frame = std::vector<std::shared_ptr<void>>;

	std::array<Frame, buffering_v> m_queue{};
	std::mutex m_mutex{};
};
} // namespace bave::detail
