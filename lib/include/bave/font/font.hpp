#pragma once
#include <bave/core/not_null.hpp>
#include <bave/font/font_atlas.hpp>
#include <bave/font/font_library.hpp>
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/rgba.hpp>
#include <optional>

namespace bave {
class Font {
  public:
	class Pen;

	static auto try_make(NotNull<RenderDevice*> render_device, std::vector<std::byte> file_bytes, float scale = 2.0f) -> std::optional<Font>;

	explicit Font(NotNull<RenderDevice*> render_device, std::unique_ptr<GlyphSlot::Factory> slot_factory, float scale);

	[[nodiscard]] auto get_font_atlas(TextHeight height) -> FontAtlas&;

	[[nodiscard]] auto glyph_for(TextHeight height, Codepoint codepoint) -> Glyph { return get_font_atlas(height).glyph_for(codepoint); }

  private:
	NotNull<RenderDevice*> m_render_device;
	std::unique_ptr<GlyphSlot::Factory> m_slot_factory{};
	float m_scale{};
	std::unordered_map<TextHeight, FontAtlas> m_atlases{};
};

class Font::Pen {
  public:
	Pen(NotNull<Font*> font, TextHeight height = TextHeight::eDefault, float scale = 1.0f)
		: m_font(font), m_height(clamp_text_height(height)), m_scale(scale) {}

	auto advance(std::string_view line) -> Pen&;
	auto generate_quads(std::string_view line) -> Geometry;
	auto generate_quads(Geometry& out, std::string_view line) -> Pen&;

	[[nodiscard]] auto calc_line_extent(std::string_view line) const -> glm::vec2;

	[[nodiscard]] auto get_texture() const -> std::shared_ptr<Texture const> const& { return m_font->get_font_atlas(m_height).get_texture(); }
	[[nodiscard]] auto get_height() const -> TextHeight { return m_height; }

	glm::vec2 cursor{};
	Rgba vertex_colour{white_v};

  private:
	struct Writer;

	NotNull<Font*> m_font;
	TextHeight m_height{};
	float m_scale{};
};
} // namespace bave
