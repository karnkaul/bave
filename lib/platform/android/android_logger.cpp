#include <android/log.h>
#include <bave/logger.hpp>

namespace bave::log {
void internal::log_message(char level, CString tag, CString message) {
	auto lvl = ANDROID_LOG_INFO;
	switch (level) {
	case error_v: lvl = ANDROID_LOG_ERROR; break;
	case warn_v: lvl = ANDROID_LOG_WARN; break;
	case debug_v: lvl = ANDROID_LOG_DEBUG; break;
	default: break;
	}

	auto logcat_out = format_message(level, tag, message, true);
	logcat_out.pop_back(); // remove newline

	__android_log_print(lvl, tag.c_str(), "%s", logcat_out.c_str());
}
} // namespace bave::log
