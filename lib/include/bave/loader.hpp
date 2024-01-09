#pragma once
#include <bave/audio/audio_clip.hpp>
#include <bave/data_store.hpp>
#include <bave/font/font.hpp>
#include <bave/graphics/image_file.hpp>
#include <bave/graphics/render_device.hpp>
#include <bave/graphics/sprite_animation.hpp>
#include <bave/graphics/sprite_sheet.hpp>
#include <bave/graphics/texture.hpp>
#include <djson/json.hpp>
#include <memory>

namespace bave {
class Loader {
  public:
	explicit Loader(NotNull<DataStore const*> data_store, NotNull<RenderDevice*> render_device);

	[[nodiscard]] auto load_bytes(std::string_view uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto load_json(std::string_view uri) const -> dj::Json;

	[[nodiscard]] auto load_image_file(std::string_view uri) const -> std::shared_ptr<ImageFile>;
	[[nodiscard]] auto load_texture(std::string_view uri, bool mip_map = false) const -> std::shared_ptr<Texture>;
	[[nodiscard]] auto load_sliced_texture(std::string_view uri) const -> std::shared_ptr<SlicedTexture>;
	[[nodiscard]] auto load_font(std::string_view uri) const -> std::shared_ptr<Font>;
	[[nodiscard]] auto load_audio_clip(std::string_view uri) const -> std::shared_ptr<AudioClip>;
	[[nodiscard]] auto load_sprite_sheet(std::string_view uri) const -> std::shared_ptr<SpriteSheet>;
	[[nodiscard]] auto load_sprite_animation(std::string_view uri) const -> std::shared_ptr<SpriteAnimation>;

  private:
	Logger m_log{"Loader"};
	NotNull<DataStore const*> m_data_store;
	NotNull<RenderDevice*> m_render_device;
};
} // namespace bave
