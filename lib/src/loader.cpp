#include <bave/graphics/image_file.hpp>
#include <bave/json_io.hpp>
#include <bave/loader.hpp>
#include <filesystem>

namespace bave {
namespace fs = std::filesystem;

namespace {
constexpr auto get_compression(std::string_view const extension) {
	if (extension == ".wav") { return Compression::eWav; }
	if (extension == ".mp3") { return Compression::eMp3; }
	if (extension == ".flac") { return Compression::eFlac; }
	return Compression::eUnknown;
}

auto make_sprite_blocks(glm::ivec2 const size, dj::Json const& json) {
	auto ret = std::vector<SpriteSheet::Block>{};
	if (auto const& map = json["map"]) {
		for (auto const& [id, rect] : map.object_view()) {
			if (id.empty()) { continue; }
			auto block = SpriteSheet::Block{.id = std::string{id}};
			from_json(rect, block.rect);
			ret.push_back(std::move(block));
		}
	} else if (auto const& id_table = json["id_table"]) {
		auto tile_count = glm::ivec2{};
		tile_count.x = static_cast<int>(id_table[0].array_view().size());
		tile_count.y = static_cast<int>(id_table.array_view().size());
		auto const rects = SpriteSheet::make_rects(size, tile_count);
		if (!rects.empty()) {
			auto index = std::size_t{};
			for (auto const& id_row : id_table.array_view()) {
				for (auto const& id : id_row.array_view()) {
					auto tile = SpriteSheet::Block{.id = std::string{id.as_string()}};
					if (tile.id.empty()) { continue; }
					tile.rect = rects.at(index);
					ret.push_back(std::move(tile));
					++index;
				}
			}
		}
	}
	return ret;
}
} // namespace

Loader::Loader(NotNull<DataStore const*> data_store, NotNull<RenderDevice*> render_device) : m_data_store(data_store), m_render_device(render_device) {}

auto Loader::load_bytes(std::string_view const uri) const -> std::vector<std::byte> {
	if (uri.empty()) {
		m_log.warn("empty URI");
		return {};
	}
	auto ret = m_data_store->read_bytes(uri);
	if (ret.empty()) {
		m_log.warn("failed to load bytes: '{}'", uri);
		return {};
	}
	return ret;
}

auto Loader::load_json(std::string_view const uri) const -> dj::Json {
	if (uri.empty()) {
		m_log.warn("empty URI");
		return {};
	}
	auto ret = m_data_store->read_json(uri);
	if (!ret) {
		m_log.warn("failed to load JSON: '{}'", uri);
		return {};
	}
	return ret;
}

auto Loader::load_image_file(std::string_view uri) const -> std::shared_ptr<ImageFile> {
	auto const bytes = load_bytes(uri);
	if (bytes.empty()) { return {}; }

	auto ret = std::make_shared<ImageFile>();
	if (!ret->load_from_bytes(bytes)) {
		m_log.warn("failed to load ImageFile: '{}'", uri);
		return {};
	}

	m_log.info("loaded ImageFile: '{}'", uri);
	return ret;
}

auto Loader::load_texture(std::string_view const uri, bool const mip_map) const -> std::shared_ptr<Texture> {
	auto const bytes = load_bytes(uri);
	if (bytes.empty()) { return {}; }

	auto image_file = ImageFile{};
	if (!image_file.load_from_bytes(bytes)) {
		m_log.warn("failed to load ImageFile: '{}'", uri);
		return {};
	}

	auto ret = std::make_shared<Texture>(m_render_device, image_file.get_bitmap_view(), mip_map);
	m_log.info("loaded Texture: '{}'", uri);
	return ret;
}

auto Loader::load_sliced_texture(std::string_view const uri) const -> std::shared_ptr<SlicedTexture> {
	auto json = load_json(uri);
	if (!json || !json.contains("image") || !json.contains("nine_slice")) { return {}; }

	auto image = load_image_file(json["image"].as_string());
	if (!image) { return {}; }

	auto slice = NineSlice{};
	from_json(json["nine_slice"], slice);

	auto ret = std::make_shared<SlicedTexture>(m_render_device, image->get_bitmap_view(), slice);
	m_log.info("loaded SlicedTexture: '{}'", uri);
	return ret;
}

auto Loader::load_font(std::string_view const uri) const -> std::shared_ptr<Font> {
	auto const bytes = load_bytes(uri);
	if (bytes.empty()) { return {}; }

	auto ret = std::make_shared<Font>(m_render_device);
	if (!ret->load_from_bytes(bytes)) {
		m_log.warn("failed to load Font: '{}'", uri);
		return {};
	}

	m_log.info("loaded Font: '{}'", uri);
	return ret;
}

auto Loader::load_audio_clip(std::string_view const uri) const -> std::shared_ptr<AudioClip> {
	auto const bytes = load_bytes(uri);
	if (bytes.empty()) { return {}; }

	auto const compression = get_compression(fs::path{uri}.extension().string());
	auto ret = std::make_shared<AudioClip>();
	if (!ret->load_from_bytes(bytes, compression)) {
		m_log.warn("failed to load AudioClip: '{}'", uri);
		return {};
	}

	m_log.info("loaded AudioClip: '{}'", uri);
	return ret;
}

auto Loader::load_sprite_sheet(std::string_view const uri) const -> std::shared_ptr<SpriteSheet> {
	auto const json = load_json(uri);
	if (!json) { return {}; }

	auto texture = load_texture(json["image"].as_string());
	if (!texture) { return {}; }

	auto blocks = make_sprite_blocks(texture->get_size(), json);

	auto ret = std::make_shared<SpriteSheet>(std::move(texture), std::move(blocks));
	m_log.info("loaded SpriteSheet: '{}'", uri);
	return ret;
}

auto Loader::load_sprite_animation(std::string_view const uri) const -> std::shared_ptr<SpriteAnimation> {
	auto const json = load_json(uri);
	if (!json) { return {}; }

	auto const duration = Seconds{json["duration"].as<float>()};
	auto const& in_key_frames = json["key_frames"];
	auto ret = std::shared_ptr<SpriteAnimation>{};
	if (!in_key_frames.array_view().empty()) {
		if (in_key_frames[0].is_object()) {
			auto key_frames = std::vector<SpriteAnimation::KeyFrame>{};
			for (auto const& in_kf : in_key_frames.array_view()) {
				auto out_kf = SpriteAnimation::KeyFrame{};
				out_kf.tile_id = std::string{in_kf["tile_id"].as_string()};
				out_kf.timestamp = Seconds{in_kf["timestamp"].as<float>()};
				if (!out_kf.tile_id.empty()) { key_frames.push_back(std::move(out_kf)); }
			}
			ret = std::make_shared<SpriteAnimation>(std::move(key_frames), duration);
		} else {
			auto tile_ids = std::vector<std::string>{};
			for (auto const& in_tile_id : in_key_frames.array_view()) { tile_ids.emplace_back(in_tile_id.as_string()); }
			ret = std::make_shared<SpriteAnimation>(std::move(tile_ids), duration);
		}
	}

	m_log.info("loaded SpriteAnimation: '{}'", uri);
	return ret;
}
} // namespace bave
