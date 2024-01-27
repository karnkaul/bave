#pragma once
#include <bave/core/make_bitset.hpp>

namespace bave {
namespace mod {
constexpr size_t none{0};
constexpr size_t shift{1};
constexpr size_t ctrl{2};
constexpr size_t alt{3};
constexpr size_t super{4};
constexpr size_t capslock{5};
constexpr size_t numlock{6};
constexpr size_t count_v{numlock + 1};
}; // namespace mod

/// \brief Bit flags for modifier keys.
using KeyMods = std::bitset<mod::count_v>;

/// \brief Create KeyMods with mods set.
/// \param mods mods to set.
/// \returns KeyMods with mods set.
template <std::convertible_to<std::size_t>... I>
constexpr auto make_key_mods(I const... mods) -> KeyMods {
	return make_bitset<KeyMods>(mods...);
}
} // namespace bave
