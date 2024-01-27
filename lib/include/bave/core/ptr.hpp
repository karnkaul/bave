#pragma once

namespace bave {
/// \brief Alias for a pointer.
///
/// `Ptr<T>` must be treated as a pointer to a single object (whereas `T*` is ambiguous).
template <typename Type>
using Ptr = Type*;
} // namespace bave
