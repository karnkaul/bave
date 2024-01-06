#pragma once
#include <bave/core/not_null.hpp>
#include <bave/core/pinned.hpp>
#include <bave/core/ptr.hpp>
#include <bave/graphics/detail/buffering.hpp>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

namespace bave::detail {
class DeferQueue : public Pinned {
  public:
	template <typename Type>
	void push(Type obj) {
		auto model = std::make_unique<Model<Type>>(std::move(obj));
		auto lock = std::scoped_lock{m_mutex};
		m_current.push_back(std::move(model));
	}

	void next_frame();
	void clear();

  private:
	struct Base { // NOLINT(cppcoreguidelines-special-member-functions)
		virtual ~Base() = default;
	};
	template <typename T>
	struct Model : Base {
		T t;
		Model(T&& t) : t(std::move(t)) {}
	};
	using Frame = std::vector<std::unique_ptr<Base>>;

	Frame m_current{};
	std::deque<Frame> m_deferred{};
	std::mutex m_mutex{};
};
} // namespace bave::detail
