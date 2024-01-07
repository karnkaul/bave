#pragma once
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/rect.hpp>
#include <bave/graphics/rgba.hpp>
#include <djson/json.hpp>

namespace bave {
template <dj::Numeric Type>
void to_json(dj::Json& json, glm::tvec2<Type> const& in) {
	json.push_back(in.x);
	json.push_back(in.y);
}

template <dj::Numeric Type>
void from_json(dj::Json const& in, glm::tvec2<Type>& out) {
	if (in.is_number()) {
		out = glm::tvec2<Type>{in.as<Type>()};
	} else {
		out = {in[0].as<Type>(), in[1].as<Type>()};
	}
}

template <dj::Numeric Type>
void to_json(dj::Json& out, Rect<Type> const& in) {
	to_json(out["lt"], in.lt);
	to_json(out["rb"], in.rb);
}

template <dj::Numeric Type>
void from_json(dj::Json const& json, Rect<Type>& out) {
	from_json(json["lt"], out.lt);
	from_json(json["rb"], out.rb);
}

void to_json(dj::Json& out, Rgba const& rgba);
void from_json(dj::Json const& json, Rgba& out);

void to_json(dj::Json& out, NineSlice const& nine_slice);
void from_json(dj::Json const& json, NineSlice& out);
} // namespace bave
