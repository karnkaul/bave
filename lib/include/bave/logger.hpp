#pragma once
#include <fmt/format.h>
#include <bave/core/c_string.hpp>

namespace bave {
namespace log {
constexpr auto error_v{'E'};
constexpr auto warn_v{'W'};
constexpr auto info_v{'I'};
constexpr auto debug_v{'D'};

[[nodiscard]] auto format_message(char level, std::string_view tag, std::string_view message) -> std::string;

void log_message(char level, CString tag, CString message);

namespace internal {
void log_message(char level, CString tag, CString message, std::string formatted);
}
} // namespace log

struct Logger {
	std::string tag{"default"};

	void log(char level, CString message) const;
};
} // namespace bave
