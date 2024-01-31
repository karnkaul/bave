#pragma once
#include <type_traits>

namespace bave {
template <typename Type>
concept EnumT = std::is_enum_v<Type>;
}
