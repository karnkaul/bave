#include <bave/json_io.hpp>

void bave::to_json(dj::Json& json, SpriteMap const& in) {
	for (auto const& in_block : in.blocks) {
		if (in_block.id.empty()) { continue; }
		to_json(json[in_block.id], in_block.rect);
	}
}

void bave::from_json(dj::Json const& json, SpriteMap& out) {
	for (auto const& [id, rect] : json.object_view()) {
		if (id.empty()) { continue; }
		auto block = SpriteSheet::Block{.id = std::string{id}};
		from_json(rect, block.rect);
		if (block.id.empty()) { continue; }
		out.blocks.push_back(std::move(block));
	}
}
