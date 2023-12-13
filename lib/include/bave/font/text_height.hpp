#pragma once

namespace bave {
enum struct TextHeight : int {
	eMin = 10,
	eDefault = 40,
	eMax = 256,
};

constexpr auto clamp_text_height(TextHeight in) -> TextHeight {
	if (in < TextHeight::eMin) { return TextHeight::eMin; }
	if (in > TextHeight::eMax) { return TextHeight::eMax; }
	return in;
}

constexpr auto scale_text_height(TextHeight const in, float const scale) -> TextHeight {
	return clamp_text_height(static_cast<TextHeight>(static_cast<float>(in) * scale));
}
} // namespace bave
