#include <fmt/format.h>
#include <bave/graphics/rgba.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/mat4x4.hpp>
#include <charconv>

namespace bave {
namespace {
[[nodiscard]] auto from_hex(std::string_view const hex) -> std::uint8_t {
	auto ret = std::uint8_t{};
	auto const* end = hex.data() + hex.size();
	auto const [ptr, ec] = std::from_chars(hex.data(), end, ret, 16);
	if (ptr != end || ec != std::errc{}) { return {}; }
	return ret;
}
} // namespace

auto Rgba::to_srgb(glm::vec4 const& linear) -> glm::vec4 { return glm::convertLinearToSRGB(linear); }
auto Rgba::to_linear(glm::vec4 const& srgb) -> glm::vec4 { return glm::convertSRGBToLinear(srgb); }

auto Rgba::from(std::string_view hex) -> Rgba {
	if (!hex.empty() && hex.front() == '#') { hex = hex.substr(1); }
	static constexpr std::string_view hex_v{"ffffffff"};
	if (hex.size() != hex_v.size()) { return {}; }

	auto out = Rgba{};
	out.channels.x = from_hex(hex.substr(0, 2));
	out.channels.y = from_hex(hex.substr(2, 2));
	out.channels.z = from_hex(hex.substr(4, 2));
	out.channels.w = from_hex(hex.substr(6));
	return out;
}

auto Rgba::to_hex_str() const -> std::string { return fmt::format("#{:x}{:x}{:x}{:x}", channels.x, channels.y, channels.z, channels.w); }
} // namespace bave
