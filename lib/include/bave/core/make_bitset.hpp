#pragma once
#include <bitset>

namespace bave {
/// \brief Create a std::bitset with bits set.
/// \param bits bits to set.
/// \returns std::bitset<Size> with bits set.
template <std::size_t Size, std::convertible_to<std::size_t>... I>
auto make_bitset(I const... bits) -> std::bitset<Size> {
	auto ret = std::bitset<Size>{};
	(ret.set(bits), ...);
	return ret;
}

/// \brief Create a std::bitset with bits set.
/// \param bits bits to set.
/// \returns Ret with bits set.
template <typename Ret, std::convertible_to<std::size_t>... I>
auto make_bitset(I const... bits) -> Ret {
	return make_bitset<Ret{}.size()>(bits...);
}
} // namespace bave
