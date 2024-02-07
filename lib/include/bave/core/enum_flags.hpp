#pragma once
#include <bave/core/enum_t.hpp>
#include <bitset>

namespace bave {
/// \brief Enum-indexed wrapper around std::bitset.
template <EnumT E, std::size_t Size = std::size_t(E::eCOUNT_)>
class EnumFlags : public std::bitset<Size> {
  public:
	using std::bitset<Size>::bitset;

	/// \brief Constructor taking multiple flags.
	/// \param to_set flags to set.
	template <std::same_as<E>... T>
	EnumFlags(T const... to_set) {
		(this->set(to_set), ...);
	}

	/// \brief Test if a flag is set.
	/// \param e Flag to test for.
	/// \returns true if set.
	[[nodiscard]] auto test(E const e) const -> bool { return std::bitset<Size>::test(static_cast<std::size_t>(e)); }
	/// \brief Set a flag.
	/// \param e Flag to set.
	void set(E const e) { std::bitset<Size>::set(static_cast<std::size_t>(e)); }
	/// \brief Reset a flag.
	/// \param e Flag to reset.
	void reset(E const e) { std::bitset<Size>::reset(static_cast<std::size_t>(e)); }

	[[nodiscard]] auto operator[](E const e) -> decltype(auto) { return std::bitset<Size>::operator[](static_cast<std::size_t>(e)); }
	[[nodiscard]] auto operator[](E const e) const -> decltype(auto) { return std::bitset<Size>::operator[](static_cast<std::size_t>(e)); }
};
} // namespace bave
