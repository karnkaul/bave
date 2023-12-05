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

	template <typename... Args>
	void error(fmt::format_string<Args...> fmt, Args&&... args) {
		log(log::error_v, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}

	template <typename... Args>
	void warn(fmt::format_string<Args...> fmt, Args&&... args) {
		log(log::warn_v, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}

	template <typename... Args>
	void info(fmt::format_string<Args...> fmt, Args&&... args) {
		log(log::info_v, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}

	template <typename... Args>
	void debug(fmt::format_string<Args...> fmt, Args&&... args) {
		log(log::debug_v, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}
};
} // namespace bave
