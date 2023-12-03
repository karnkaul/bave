#pragma once
#include <fmt/format.h>

namespace bave {
class Error : public std::runtime_error {
  public:
	template <typename... Args>
	explicit Error(fmt::format_string<Args...> fmt, Args&&... args) : std::runtime_error{fmt::format(fmt, std::forward<Args>(args)...)} {}
};
} // namespace bave
