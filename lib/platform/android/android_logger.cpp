#include <android/log.h>
#include <bave/logger.hpp>

namespace bave::log {
void internal::log_message(char level, CString tag, CString message, std::string /*formatted*/) {
	auto lvl = ANDROID_LOG_INFO;
	switch (level) {
	case error_v: lvl = ANDROID_LOG_ERROR; break;
	case warn_v: lvl = ANDROID_LOG_WARN; break;
	case debug_v: lvl = ANDROID_LOG_DEBUG; break;
	default: break;
	}

	__android_log_print(lvl, tag.c_str(), "%s", message.c_str());
}
} // namespace bave::log
