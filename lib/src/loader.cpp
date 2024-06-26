#include <bave/graphics/image_file.hpp>
#include <bave/io/json_io.hpp>
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

auto Loader::load_json_asset(std::string_view const uri, std::string_view const asset_type) const -> dj::Json {
	if (asset_type.empty()) {
		m_log.warn("cannot load unknown asset type");
		return {};
	}

	auto ret = load_json(uri);
	if (!ret) { return {}; }

	if (!ret.contains("asset_type")) {
		m_log.warn("JSON missing 'asset_type' field: '{}'", uri);
		return {};
	}

	auto const in_asset_type = ret["asset_type"].as_string();
	if (in_asset_type != asset_type) {
		m_log.warn("JSON asset_type mismatch, expected: '{}', obtained: '{}'", asset_type, in_asset_type);
		return {};
	}

	return ret;
}

auto Loader::load_image_file(std::string_view uri) const -> std::optional<ImageFile> {
	auto const bytes = load_bytes(uri);
	if (bytes.empty()) { return {}; }

	auto ret = ImageFile{};
	if (!ret.load_from_bytes(bytes)) {
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

auto Loader::load_texture_9slice(std::string_view const uri) const -> std::shared_ptr<Texture9Slice> {
	auto json = load_json_asset<Texture9Slice>(uri);
	if (!json) { return {}; }

	auto image = load_image_file(json["image"].as_string());
	if (!image) { return {}; }

	auto slice = NineSlice{};
	from_json(json["nine_slice"], slice);

	auto ret = std::make_shared<Texture9Slice>(m_render_device, image->get_bitmap_view(), slice);
	m_log.info("loaded Texture9Slice: '{}'", uri);
	return ret;
}

auto Loader::load_texture_atlas(std::string_view uri, bool mip_map) const -> std::shared_ptr<TextureAtlas> {
	auto json = load_json_asset<TextureAtlas>(uri);
	if (!json) { return {}; }

	auto image = load_image_file(json["image"].as_string());
	if (!image) { return {}; }

	auto sheet = TileSheet{};
	from_json(json["tile_sheet"], sheet);

	auto ret = std::make_shared<TextureAtlas>(m_render_device, image->get_bitmap_view(), sheet, mip_map);
	m_log.info("loaded TextureAtlas: '{}'", uri);
	return ret;
}

auto Loader::load_font(std::string_view const uri, std::span<TextHeight const> preload) const -> std::shared_ptr<Font> {
	auto const bytes = load_bytes(uri);
	if (bytes.empty()) { return {}; }

	auto ret = std::make_shared<Font>(m_render_device);
	if (!ret->load_from_bytes(bytes)) {
		m_log.warn("failed to load Font: '{}'", uri);
		return {};
	}

	for (auto const height : preload) { [[maybe_unused]] auto const glyph = ret->glyph_for(height, {}); }

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

auto Loader::load_anim_timeline(std::string_view const uri) const -> std::shared_ptr<AnimTimeline> {
	auto const json = load_json_asset<AnimTimeline>(uri);
	if (!json) { return {}; }

	auto ret = std::make_shared<AnimTimeline>();
	ret->duration = Seconds{json["duration"].as<float>()};
	auto const& in_tiles = json["tiles"];
	ret->tiles.reserve(in_tiles.array_view().size());
	for (auto const& in_tile : in_tiles.array_view()) {
		auto tile_id = in_tile.as_string();
		if (tile_id.empty()) { continue; }
		ret->tiles.emplace_back(tile_id);
	}

	m_log.info("loaded AnimTimeline: '{}'", uri);
	return ret;
}

auto Loader::load_particle_emitter(std::string_view const uri) const -> std::shared_ptr<ParticleEmitter> {
	auto const json = load_json_asset<ParticleEmitter>(uri);
	if (!json) { return {}; }

	auto ret = std::make_shared<ParticleEmitter>();
	from_json(json["config"], ret->config);

	if (auto const& modifiers = json["modifiers"]) {
		ret->modifiers = {};
		for (auto const& modifier : modifiers.array_view()) {
			if (modifier.as_string() == "translate") {
				ret->modifiers.set(ParticleEmitter::Modifier::eTranslate);
			} else if (modifier.as_string() == "rotate") {
				ret->modifiers.set(ParticleEmitter::Modifier::eRotate);
			} else if (modifier.as_string() == "scale") {
				ret->modifiers.set(ParticleEmitter::Modifier::eScale);
			} else if (modifier.as_string() == "tint") {
				ret->modifiers.set(ParticleEmitter::Modifier::eTint);
			}
		}
	}

	if (auto const& texture = json["texture"]) { ret->set_texture(load_texture(texture.as_string())); }

	m_log.info("loaded ParticleEmitter: '{}'", uri);
	return ret;
}
} // namespace bave
