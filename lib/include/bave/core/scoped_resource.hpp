#pragma once
#include <concepts>
#include <utility>

namespace bave {
template <std::equality_comparable Type, typename Deleter>
	requires(std::is_invocable_v<Deleter, Type>)
class ScopedResource {
  public:
	using value_type = Type;

	ScopedResource(ScopedResource const&) = delete;
	auto operator=(ScopedResource const&) -> ScopedResource& = delete;

	ScopedResource() = default;

	constexpr ScopedResource(Type t, Deleter deleter = Deleter{}) : m_t(std::move(t)), m_deleter(std::move(deleter)) {}

	constexpr ScopedResource(ScopedResource&& rhs) noexcept { swap(*this, rhs); }
	constexpr auto operator=(ScopedResource&& rhs) noexcept -> ScopedResource& {
		if (&rhs != this) { swap(*this, rhs); }
		return *this;
	}

	constexpr ~ScopedResource() noexcept {
		if (m_t == Type{}) { return; }
		m_deleter(m_t);
	}

	constexpr auto get() const -> Type const& { return m_t; }
	constexpr auto get() -> Type& { return m_t; }

	constexpr operator Type const&() const { return get(); }
	constexpr operator Type&() { return get(); }

	explicit constexpr operator bool() const { return m_t != Type{}; }

	friend constexpr void swap(ScopedResource& lhs, ScopedResource& rhs) noexcept {
		std::swap(lhs.m_t, rhs.m_t);
		std::swap(lhs.m_deleter, rhs.m_deleter);
	}

  private:
	Type m_t{};
	[[no_unique_address]] Deleter m_deleter{};
};
} // namespace bave
