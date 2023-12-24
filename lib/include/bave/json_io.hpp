#pragma once
#include <bave/graphics/sprite_sheet.hpp>
#include <djson/json.hpp>

namespace bave {
struct SpriteMap {
	std::vector<SpriteSheet::Block> blocks{};
};

template <dj::Numeric Type>
void to_json(dj::Json& json, glm::tvec2<Type> const& in) {
	json.push_back(in.x);
	json.push_back(in.y);
}

template <dj::Numeric Type>
void from_json(dj::Json const& in, glm::tvec2<Type>& out) {
	out = {in[0].as<Type>(), in[1].as<Type>()};
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

void to_json(dj::Json& json, SpriteMap const& in);
void from_json(dj::Json const& json, SpriteMap& out);
} // namespace bave
