#pragma once
#include <bave/input/key.hpp>
#include <bitset>

namespace bave {
/// \brief Stateful wrapper for keys held down over multiple frames.
struct KeyState {
	static constexpr auto max_keys_v = static_cast<std::size_t>(Key::eCOUNT_);

	std::bitset<max_keys_v> held_keys{};

	/// \brief Check if a key is held down (pressed).
	/// \param key The key to check for.
	/// \returns true if key has been pressed but not yet released.
	/// \pre key must be non-negative and less than Key::eCOUNT_.
	[[nodiscard]] auto is_held(Key const key) const -> bool { return held_keys.test(static_cast<std::size_t>(key)); }
};
} // namespace bave
