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

void bave::to_json(dj::Json& out, TileSheet const& tile_sheet) {
	for (auto const& in_tile : tile_sheet.tiles) {
		if (in_tile.id.empty()) { continue; }
		auto& out_tile = out.push_back({});
		out_tile["id"] = in_tile.id;
		to_json(out_tile["image_rect"], in_tile.image_rect);
		if (!in_tile.colliders.empty()) {
			auto& out_colliders = out_tile["colliders"];
			for (auto const& in_collider : in_tile.colliders) { to_json(out_colliders.push_back({}), in_collider); }
		}
	}
}

void bave::from_json(dj::Json const& json, TileSheet& out) {
	for (auto const& in_tile : json["tiles"].array_view()) {
		auto out_tile = TileSheet::Tile{};
		out_tile.id = in_tile["id"].as_string();
		if (out_tile.id.empty()) { continue; }
		from_json(in_tile["image_rect"], out_tile.image_rect);
		for (auto const& in_collider : in_tile["colliders"].array_view()) {
			auto out_collider = Rect<int>{};
			from_json(in_collider, out_collider);
			out_tile.colliders.push_back(out_collider);
		}
		out.tiles.push_back(std::move(out_tile));
	}
}
