#pragma once
#include <bitset>

namespace bave {
namespace mod {
constexpr size_t none{0};
constexpr size_t shift{1};
constexpr size_t ctrl{2};
constexpr size_t alt{3};
constexpr size_t super{4};
constexpr size_t capslock{5};
constexpr size_t numlock{6};
constexpr size_t count_v{numlock};
}; // namespace mod

///
/// \brief Bit flags for modifier keys.
///
using KeyMods = std::bitset<mod::count_v>;

///! Create KeyMods with mods set.
template <std::convertible_to<std::size_t>... I>
constexpr auto make_key_mods(I const... mods) -> KeyMods {
	auto ret = KeyMods{};
	(ret.set(mods), ...);
	return ret;
}
} // namespace bave
