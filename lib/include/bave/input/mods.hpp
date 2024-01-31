#pragma once
#include <bave/core/make_bitset.hpp>

namespace bave {
enum class Mod : int { eShift, eCtrl, eAlt, eSuper, eCapsLock, eNumLock, eCOUNT_ };

/// \brief Bit flags for modifier keys.
using KeyMods = std::bitset<static_cast<std::size_t>(Mod::eCOUNT_)>;

/// \brief Create KeyMods with mods set.
/// \param mods mods to set.
/// \returns KeyMods with mods set.
template <std::same_as<Mod>... I>
auto make_key_mods(I const... mods) -> KeyMods {
	return make_bitset<KeyMods>(static_cast<std::size_t>(mods)...);
}
} // namespace bave
