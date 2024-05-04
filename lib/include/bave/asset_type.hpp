#pragma once
#include <concepts>
#include <string_view>

namespace bave {
struct AnimTimeline;
class Texture9Slice;
class TextureAtlas;
class ParticleEmitter;

namespace detail {
template <typename...>
constexpr auto false_v = false;
}

template <typename Type>
constexpr auto get_asset_type() -> std::string_view {
	if constexpr (std::same_as<Type, AnimTimeline>) {
		return "AnimTimeline";
	} else if constexpr (std::same_as<Type, Texture9Slice>) {
		return "Texture9Slice";
	} else if constexpr (std::same_as<Type, TextureAtlas>) {
		return "TextureAtlas";
	} else if constexpr (std::same_as<Type, ParticleEmitter>) {
		return "ParticleEmitter";
	}

	else {
		static_assert(detail::false_v<Type>, "unsupported asset type");
	}
}
} // namespace bave
