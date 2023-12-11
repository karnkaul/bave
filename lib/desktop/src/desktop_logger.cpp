#include <fmt/chrono.h>
#include <bave/logger.hpp>
#include <iostream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace bave::log {
void internal::log_message(char level, CString /*tag*/, CString /*message*/, std::string formatted) {
	formatted += '\n';

	auto& out = level == error_v ? std::cerr : std::cout;
	out << formatted.c_str();

#if defined(_WIN32)
	OutputDebugStringA(formatted.c_str());
#endif
}
} // namespace bave::log
