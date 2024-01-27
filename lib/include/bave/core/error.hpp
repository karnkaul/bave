#pragma once
#include <fmt/format.h>

namespace bave {
/// \brief Base class for all errors thrown within bave.
class Error : public std::runtime_error {
  public:
	/// \brief Constructor.
	/// \param fmt Format string.
	/// \param args Arguments.
	template <typename... Args>
	explicit Error(fmt::format_string<Args...> fmt, Args&&... args) : std::runtime_error{fmt::format(fmt, std::forward<Args>(args)...)} {}
};
} // namespace bave
