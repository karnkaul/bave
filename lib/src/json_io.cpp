#include <bave/json_io.hpp>

void bave::to_json(dj::Json& out, Rgba const& rgba) { out = rgba.to_hex_str(); }

void bave::from_json(dj::Json const& json, Rgba& out) { out = Rgba::from(json.as_string()); }
