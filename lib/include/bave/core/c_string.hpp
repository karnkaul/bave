#pragma once
#include <string_view>

namespace bave {
/// \brief Wrapper over a C string (char const*).
class CString {
  public:
	CString() = default;

	/// \brief Implicit constructor.
	/// \param text string to store.
	/*implicit*/ constexpr CString(char const* text) : text(text) {}

	/// \brief View as C string.
	/// \returns const pointer to first char (of empty string by default, not nullptr).
	[[nodiscard]] constexpr auto c_str() const -> char const* { return text; }
	/// \brief View as string_view.
	/// \returns std::string_view into stored string.
	[[nodiscard]] constexpr auto as_view() const -> std::string_view { return c_str(); }

	constexpr operator std::string_view() const { return as_view(); }
	explicit constexpr operator bool() const { return text != nullptr; }

  private:
	char const* text{""};
};
} // namespace bave
