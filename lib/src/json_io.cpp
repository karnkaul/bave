#include <bave/json_io.hpp>

namespace {
void to_json(dj::Json& out, bave::Radians const& radians) { out = radians.to_degrees().value; }
void from_json(dj::Json const& json, bave::Radians& out) { out = bave::Degrees{json.as<float>()}; }

void to_json(dj::Json& out, bave::Seconds const& seconds) { out = seconds.count(); }
void from_json(dj::Json const& json, bave::Seconds& out) { out = bave::Seconds{json.as<float>()}; }

template <typename Type>
void to_json(dj::Json& out, bave::InclusiveRange<Type> const& range) {
	using ::to_json;
	using bave::to_json;
	to_json(out["lo"], range.lo);
	to_json(out["hi"], range.hi);
}

template <typename Type>
void from_json(dj::Json const& json, bave::InclusiveRange<Type>& out) {
	using ::from_json;
	using bave::from_json;
	from_json(json["lo"], out.lo);
	from_json(json["hi"], out.hi);
}
} // namespace

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

void bave::to_json(dj::Json& out, ParticleConfig const& particle_config) {
	using ::to_json;
	using bave::to_json;

	to_json(out["initial"]["position"], particle_config.initial.position);
	to_json(out["initial"]["rotation"], particle_config.initial.rotation.lo);

	to_json(out["velocity"]["linear"]["angle"], particle_config.velocity.linear.angle);
	to_json(out["velocity"]["linear"]["speed"], particle_config.velocity.linear.speed);
	to_json(out["velocity"]["angular"], particle_config.velocity.angular);

	to_json(out["lerp"]["tint"], particle_config.lerp.tint);
	to_json(out["lerp"]["scale"], particle_config.lerp.scale);

	to_json(out["ttl"], particle_config.ttl);
	to_json(out["quad_size"], particle_config.quad_size);
	to_json(out["count"], particle_config.count);
	to_json(out["respawn"], particle_config.respawn);
}

void bave::from_json(dj::Json const& json, ParticleConfig& out) {
	using ::from_json;
	using bave::from_json;

	from_json(json["initial"]["position"], out.initial.position);
	from_json(json["initial"]["rotation"], out.initial.rotation.lo);

	from_json(json["velocity"]["linear"]["angle"], out.velocity.linear.angle);
	from_json(json["velocity"]["linear"]["speed"], out.velocity.linear.speed);
	from_json(json["velocity"]["angular"], out.velocity.angular);

	from_json(json["lerp"]["tint"], out.lerp.tint);
	from_json(json["lerp"]["scale"], out.lerp.scale);

	from_json(json["ttl"], out.ttl);
	from_json(json["quad_size"], out.quad_size);
	from_json(json["count"], out.count);
	from_json(json["respawn"], out.respawn);
}
