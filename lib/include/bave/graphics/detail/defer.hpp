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
template <typename Type>
class Defer;

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

	template <typename Type>
	[[nodiscard]] auto make_deferred(Type t) -> Defer<Type> {
		return Defer<Type>{this, std::move(t)};
	}

  private:
	struct Base { // NOLINT
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

template <typename Type>
class Defer {
  public:
	Defer(Defer const&) = delete;
	auto operator=(Defer const&) -> Defer& = delete;

	Defer(Defer&& rhs) noexcept { swap(*this, rhs); }

	auto operator=(Defer&& rhs) noexcept -> Defer& {
		if (&rhs != this) { swap(*this, rhs); }
		return *this;
	}

	Defer() = default;

	Defer(NotNull<DeferQueue*> queue, Type obj) : m_queue(queue), m_t(std::move(obj)) {}

	~Defer() {
		if (m_queue == nullptr) { return; }
		m_queue->push(std::move(m_t));
	}

	auto get() const -> Type const& { return m_t; }
	auto get() -> Type& { return m_t; }

	operator Type const&() const { return get(); }
	operator Type&() { return get(); }

	friend void swap(Defer& lhs, Defer& rhs) noexcept {
		std::swap(lhs.m_queue, rhs.m_queue);
		std::swap(lhs.m_t, rhs.m_t);
	}

  private:
	Ptr<DeferQueue> m_queue{};
	Type m_t{};
};
} // namespace bave::detail
