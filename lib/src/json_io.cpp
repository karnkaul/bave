#include <bave/json_io.hpp>

void bave::to_json(dj::Json& out, Rgba const& rgba) { out = rgba.to_hex_str(); }

void bave::from_json(dj::Json const& json, Rgba& out) { out = Rgba::from(json.as_string()); }

void bave::to_json(dj::Json& out, NineSlice const& nine_slice) {
	auto& size = out["size"];
	to_json(size["total"], nine_slice.size.total);
	to_json(size["left_top"], nine_slice.size.left_top);
	to_json(size["right_bottom"], nine_slice.size.right_bottom);
	to_json(out["top_uv"], nine_slice.top_uv);
	to_json(out["bottom_uv"], nine_slice.bottom_uv);
	to_json(out["rgba"], nine_slice.rgba);
	to_json(out["origin"], nine_slice.origin);
}

void bave::from_json(dj::Json const& json, NineSlice& out) {
	auto const& size = json["size"];
	from_json(size["total"], out.size.total);
	from_json(size["left_top"], out.size.left_top);
	from_json(size["right_bottom"], out.size.right_bottom);
	from_json(json["top_uv"], out.top_uv);
	from_json(json["bottom_uv"], out.bottom_uv);
	from_json(json["rgba"], out.rgba);
	from_json(json["origin"], out.origin);
}
