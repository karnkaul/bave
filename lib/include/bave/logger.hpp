#pragma once
#include <fmt/format.h>
#include <bave/core/c_string.hpp>

namespace bave {
namespace log {
enum class Level : int { eError, eWarn, eInfo, eDebug };

[[nodiscard]] auto format_full(char level, std::string_view tag, std::string_view message) -> std::string;
[[nodiscard]] auto format_thread(std::string_view message) -> std::string;

void log_message(Level level, CString tag, CString message);
auto get_thread_id() -> int;

void set_max_level(Level level);
void set_max_level(std::string_view tag, Level level);

namespace internal {
void log_message(char level, CString tag, CString message);
}
} // namespace log

struct Logger {
	std::string tag{"default"};

	void log(log::Level level, CString message) const;

	template <typename... Args>
	void error(fmt::format_string<Args...> fmt, Args&&... args) const {
		log(log::Level::eError, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}

	template <typename... Args>
	void warn(fmt::format_string<Args...> fmt, Args&&... args) const {
		log(log::Level::eWarn, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}

	template <typename... Args>
	void info(fmt::format_string<Args...> fmt, Args&&... args) const {
		log(log::Level::eInfo, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}

	template <typename... Args>
	void debug(fmt::format_string<Args...> fmt, Args&&... args) const {
		log(log::Level::eDebug, fmt::format(fmt, std::forward<Args>(args)...).c_str());
	}
};
} // namespace bave
