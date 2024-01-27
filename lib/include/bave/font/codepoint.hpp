#pragma once

namespace bave {
/// \brief Open set of codepoints.
enum struct Codepoint : int {
	eTofu = 0u,
	eSpace = 32u,
	eAsciiFirst = eSpace,
	eAsciiLast = 126,
};
} // namespace bave
