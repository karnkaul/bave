#include <fmt/format.h>
#include <bave/core/version.hpp>
#include <sstream>

namespace bave {
auto Version::from(std::string text) -> std::optional<Version> {
	auto str = std::stringstream{std::move(text)};
	auto ch = char{};
	auto ret = Version{};
	if (!(str >> ch) || ch != 'v') { return {}; }
	if (!(str >> ret.major)) { return {}; }
	if (!(str >> ch) || ch != '.') { return {}; }
	if (!(str >> ret.minor)) { return {}; }
	if (!(str >> ch) || ch != '.') { return {}; }
	if (!(str >> ret.patch)) { return {}; }
	return ret;
}
} // namespace bave

auto bave::to_string(Version const& version) -> std::string { return fmt::format("v{}.{}.{}", version.major, version.minor, version.patch); }
