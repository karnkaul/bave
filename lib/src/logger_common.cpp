#include <fmt/chrono.h>
#include <bave/logger.hpp>

namespace bave {
auto log::format_message(char const level, std::string_view const tag, std::string_view const message) -> std::string {
	return fmt::format("[{}][{}] {} [{:%T}]", level, tag, message, std::chrono::system_clock::now());
}

void log::log_message(char const level, CString const tag, CString const message) {
	auto formatted = format_message(level, tag, message);

	internal::log_message(level, tag, message, std::move(formatted));
}

void Logger::log(char const level, CString const message) const { log::log_message(level, tag.c_str(), message); }
} // namespace bave
