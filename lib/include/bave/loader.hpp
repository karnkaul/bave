#pragma once
#include <bave/asset_type.hpp>
#include <bave/audio/audio_clip.hpp>
#include <bave/data_store.hpp>
#include <bave/font/font.hpp>
#include <bave/graphics/anim_timeline.hpp>
#include <bave/graphics/image_file.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/texture.hpp>
#include <bave/graphics/texture_9slice.hpp>
#include <bave/graphics/texture_atlas.hpp>
#include <djson/json.hpp>
#include <memory>

namespace bave {
class Loader {
  public:
	explicit Loader(NotNull<DataStore const*> data_store, NotNull<RenderDevice*> render_device);

	[[nodiscard]] auto load_bytes(std::string_view uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto load_json(std::string_view uri) const -> dj::Json;
	[[nodiscard]] auto load_json_asset(std::string_view uri, std::string_view asset_type) const -> dj::Json;

	template <typename Type>
	[[nodiscard]] auto load_json_asset(std::string_view const uri) const -> dj::Json {
		return load_json_asset(uri, get_asset_type<Type>());
	}

	[[nodiscard]] auto load_image_file(std::string_view uri) const -> std::optional<ImageFile>;
	[[nodiscard]] auto load_anim_timeline(std::string_view uri) const -> std::optional<AnimTimeline>;

	[[nodiscard]] auto load_texture(std::string_view uri, bool mip_map = false) const -> std::shared_ptr<Texture>;
	[[nodiscard]] auto load_texture_9slice(std::string_view uri) const -> std::shared_ptr<Texture9Slice>;
	[[nodiscard]] auto load_texture_atlas(std::string_view uri, bool mip_map = false) const -> std::shared_ptr<TextureAtlas>;
	[[nodiscard]] auto load_font(std::string_view uri) const -> std::shared_ptr<Font>;
	[[nodiscard]] auto load_audio_clip(std::string_view uri) const -> std::shared_ptr<AudioClip>;

  private:
	Logger m_log{"Loader"};
	NotNull<DataStore const*> m_data_store;
	NotNull<RenderDevice*> m_render_device;
};
} // namespace bave
