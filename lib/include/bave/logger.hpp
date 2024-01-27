#pragma once
#include <fmt/format.h>
#include <bave/core/c_string.hpp>

namespace bave {
namespace log {
/// \brief Log Level.
enum class Level : int { eError, eWarn, eInfo, eDebug };

/// \brief Format a log message.
/// \param level Log level in char form.
/// \param tag Tag / domain of logger.
/// \param message Log message.
/// \returns Formatted log string with level, thread ID, tag, message, and timestamp (terminating in `\n`).
[[nodiscard]] auto format_full(char level, std::string_view tag, std::string_view message) -> std::string;
/// \brief Add thread ID to log message.
/// \param message Log message.
/// \returns Formatted log string with thread ID and message (terminating in '\n').
[[nodiscard]] auto format_thread(std::string_view message) -> std::string;

/// \brief Log a message.
/// \param level Log level.
/// \param tag Tag / domain of logger.
/// \param message Log message.
void log_message(Level level, CString tag, CString message);

/// \brief Get this thread's log ID.
/// \returns Monotonically increasing log thread ID.
auto get_thread_id() -> int;

/// \brief Set the max logging level.
/// \param level Max level: logs with levels below this will be ignored.
void set_max_level(Level level);
/// \brief Set the max logging level for a particular tag.
/// \param level Max level: logs with this tag and levels below this will be ignored.
/// \param tag Tag to associate this rule with.
void set_max_level(std::string_view tag, Level level);

namespace internal {
void log_message(char level, CString tag, CString message);
}
} // namespace log

/// \brief Logging wrapper with stored tag.
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
