#pragma once
#include <glm/vec2.hpp>
#include <span>
#include <vector>

namespace bave {
template <std::size_t Channels>
struct BasicBitmap;

template <std::size_t Channels>
struct BasicBitmapView {
	static constexpr auto channels_v = Channels;

	std::span<std::byte const> bytes{};
	glm::ivec2 extent{};
};

using BitmapView = BasicBitmapView<4>;

template <std::size_t Channels>
struct BasicBitmap {
	static constexpr auto channels_v = Channels;

	std::vector<std::byte> bytes{};
	glm::ivec2 extent{};

	[[nodiscard]] constexpr auto view() const -> BasicBitmapView<Channels> { return {.bytes = std::span<std::byte const>{bytes}, .extent = extent}; }
};

using Bitmap = BasicBitmap<4>;
} // namespace bave
