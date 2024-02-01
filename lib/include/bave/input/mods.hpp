#pragma once
#include <bave/core/enum_flags.hpp>

namespace bave {
enum class Mod : int { eShift, eCtrl, eAlt, eSuper, eCapsLock, eNumLock, eCOUNT_ };

/// \brief Bit flags for modifier keys.
using KeyMods = EnumFlags<Mod>;
} // namespace bave
