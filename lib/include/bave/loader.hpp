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
#include <bave/graphics/tile_sheet.hpp>
#include <djson/json.hpp>
#include <memory>

namespace bave {
/// \brief Asset loader.
class Loader {
  public:
	/// \brief Constructor.
	/// \param data_store Non-null pointer to a const DataStore.
	/// \param render_device Non-null pointer to a RenderDevice.
	explicit Loader(NotNull<DataStore const*> data_store, NotNull<RenderDevice*> render_device);

	/// \brief Try to load bytes.
	/// \param uri URI to load from.
	/// \returns vector of bytes on success, empty vector on failure.
	[[nodiscard]] auto load_bytes(std::string_view uri) const -> std::vector<std::byte>;
	/// \brief Try to load a Json.
	/// \param uri URI to load from.
	/// \returns Json on success, null Json on failure.
	[[nodiscard]] auto load_json(std::string_view uri) const -> dj::Json;
	/// \brief Try to load a Json asset.
	/// \param uri URI to load from.
	/// \param asset_type Expected asset_type.
	/// \returns Json on success, null Json on failure.
	[[nodiscard]] auto load_json_asset(std::string_view uri, std::string_view asset_type) const -> dj::Json;
	/// \brief Try to load a Json asset.
	/// \param uri URI to load from.
	/// \returns Json on success, null Json on failure.
	template <typename Type>
	[[nodiscard]] auto load_json_asset(std::string_view const uri) const -> dj::Json {
		return load_json_asset(uri, get_asset_type<Type>());
	}

	/// \brief Try to load an ImageFile.
	/// \param uri URI to load from.
	/// \returns ImageFile on success, std::nullopt on failure.
	[[nodiscard]] auto load_image_file(std::string_view uri) const -> std::optional<ImageFile>;

	/// \brief Try to load a Texture.
	/// \param uri URI to load from.
	/// \param mip_map Whether to enable mip-mapping.
	/// \returns Texture on success, nullptr on failure.
	[[nodiscard]] auto load_texture(std::string_view uri, bool mip_map = false) const -> std::shared_ptr<Texture>;
	/// \brief Try to load a Texture9Slice.
	/// \param uri URI to load from.
	/// \returns Texture9Slice on success, nullptr on failure.
	[[nodiscard]] auto load_texture_9slice(std::string_view uri) const -> std::shared_ptr<Texture9Slice>;
	/// \brief Try to load a TextureAtlas.
	/// \param uri URI to load from.
	/// \param mip_map Whether to enable mip-mapping.
	/// \returns TextureAtlas on success, nullptr on failure.
	[[nodiscard]] auto load_texture_atlas(std::string_view uri, bool mip_map = false) const -> std::shared_ptr<TextureAtlas>;
	/// \brief Try to load a Font.
	/// \param uri URI to load from.
	/// \returns Font on success, nullptr on failure.
	[[nodiscard]] auto load_font(std::string_view uri) const -> std::shared_ptr<Font>;
	/// \brief Try to load an AudioClip.
	/// \param uri URI to load from.
	/// \returns AudioClip on success, nullptr on failure.
	[[nodiscard]] auto load_audio_clip(std::string_view uri) const -> std::shared_ptr<AudioClip>;
	/// \brief Try to load an AnimTimeline.
	/// \param uri URI to load from.
	/// \returns AnimTimeline on success, nullptr on failure.
	[[nodiscard]] auto load_anim_timeline(std::string_view uri) const -> std::shared_ptr<AnimTimeline>;

  private:
	Logger m_log{"Loader"};
	NotNull<DataStore const*> m_data_store;
	NotNull<RenderDevice*> m_render_device;
};
} // namespace bave
