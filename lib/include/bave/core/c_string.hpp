#pragma once
#include <string_view>

namespace bave {
class CString {
  public:
	CString() = default;

	constexpr CString(char const* text) : text(text) {}

	[[nodiscard]] constexpr auto c_str() const -> char const* { return text; }
	[[nodiscard]] constexpr auto as_view() const -> std::string_view { return c_str(); }

	constexpr operator std::string_view() const { return as_view(); }
	explicit constexpr operator bool() const { return text != nullptr; }

  private:
	char const* text{""};
};
} // namespace bave
