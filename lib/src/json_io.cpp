#include <bave/json_io.hpp>

void bave::to_json(dj::Json& out, Rgba const& rgba) { out = rgba.to_hex_str(); }

void bave::from_json(dj::Json const& json, Rgba& out) { out = Rgba::from(json.as_string()); }

void bave::to_json(dj::Json& out, NineSlice const& nine_slice) {
	to_json(out["n_left_top"], nine_slice.n_left_top);
	to_json(out["n_right_bottom"], nine_slice.n_right_bottom);
}

void bave::from_json(dj::Json const& json, NineSlice& out) {
	from_json(json["n_left_top"], out.n_left_top);
	from_json(json["n_right_bottom"], out.n_right_bottom);
}
