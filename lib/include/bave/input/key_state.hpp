#pragma once
#include <bave/core/enum_flags.hpp>
#include <bave/input/key.hpp>

namespace bave {
/// \brief Stateful wrapper for keys held down over multiple frames.
struct KeyState {
	static constexpr auto max_keys_v = static_cast<std::size_t>(Key::eCOUNT_);

	EnumFlags<Key> held_keys{};

	/// \brief Check if a key is pressed (held down).
	/// \param key The key to check for.
	/// \returns true if key has been pressed but not yet released.
	/// \pre key must be non-negative and less than Key::eCOUNT_.
	[[nodiscard]] auto is_pressed(Key const key) const -> bool { return held_keys.test(key); }

	/// \brief Check if any given key is pressed (held down).
	/// \param keys The keys to check for.
	/// \returns true if any key in keys has been pressed but not yet released.
	/// \pre keys must be non-negative and less than Key::eCOUNT_.
	template <std::same_as<Key>... Ks>
	[[nodiscard]] auto any_pressed(Ks const... keys) const -> bool {
		return held_keys.test_any(EnumFlags<Key>{keys...});
	}

	/// \brief Check if all given keys are pressed (held down).
	/// \param keys The keys to check for.
	/// \returns true if all keys have been pressed but not yet released.
	/// \pre key must be non-negative and less than Key::eCOUNT_.
	template <std::same_as<Key>... Ks>
	[[nodiscard]] auto all_pressed(Ks const... key) const -> bool {
		return held_keys.test_all(EnumFlags<Key>{key...});
	}
};
} // namespace bave
