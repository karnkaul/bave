#pragma once
#include <bave/core/offset_span.hpp>
#include <glm/vec2.hpp>

namespace bave {
template <typename StorageT, std::size_t Channels>
struct BasicBitmap {
	static constexpr auto channels_v = Channels;

	StorageT bytes{};
	glm::uvec2 extent{};
};

template <std::size_t Channels>
using BitmapByteSpan = BasicBitmap<OffsetSpan, Channels>;

template <std::size_t Channels>
using BitmapView = BasicBitmap<std::span<std::byte const>, Channels>;

using Bitmap = BitmapView<4>;

struct BitmapWrite {
	Bitmap bitmap{};
	glm::uvec2 top_left{};
};
} // namespace bave
