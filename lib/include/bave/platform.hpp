#pragma once

namespace bave {
enum class Platform { eDesktop, eAndroid };

constexpr auto platform_v =
#if defined(__ANDROID__)
	Platform::eAndroid;
#else
	Platform::eDesktop;
#endif

constexpr auto debug_v =
#if defined(BAVE_DEBUG)
	true;
#else
	false;
#endif
} // namespace bave
