#pragma once
#include <bave/core/inclusive_range.hpp>
#include <bave/core/not_null.hpp>
#include <bave/font/detail/font_atlas.hpp>
#include <bave/graphics/geometry.hpp>
#include <bave/graphics/rgba.hpp>

namespace bave {
class RenderDevice;

class Font {
  public:
	class Pen;

	static constexpr auto scale_v{2.0f};
	static constexpr auto scale_limit_v = InclusiveRange<float>{1.0f, 16.0f};

	explicit Font(NotNull<RenderDevice*> render_device);

	auto load_from_bytes(std::vector<std::byte> file_bytes, float scale = scale_v) -> bool;

	[[nodiscard]] auto glyph_for(TextHeight height, Codepoint codepoint) -> Glyph;
	[[nodiscard]] auto get_texture(TextHeight height) -> std::shared_ptr<Texture const>;

	[[nodiscard]] auto is_loaded() const -> bool { return m_slot_factory != nullptr; }

	[[nodiscard]] auto get_render_device() const -> RenderDevice& { return *m_render_device; }
	[[nodiscard]] auto get_font_atlas(TextHeight height) -> Ptr<detail::FontAtlas const>;

  private:
	NotNull<RenderDevice*> m_render_device;
	std::unique_ptr<detail::GlyphSlot::Factory> m_slot_factory{};
	float m_scale{};
	std::unordered_map<TextHeight, detail::FontAtlas> m_atlases{};
};

class Font::Pen {
  public:
	Pen(NotNull<Font*> font, TextHeight height = TextHeight::eDefault, float scale = 1.0f)
		: m_font(font), m_height(clamp_text_height(height)), m_scale(scale) {}

	auto advance(std::string_view line) -> Pen&;
	auto generate_quads(std::string_view line) -> Geometry;
	auto generate_quads(Geometry& out, std::string_view line) -> Pen&;

	[[nodiscard]] auto calc_line_extent(std::string_view line) const -> glm::vec2;

	[[nodiscard]] auto get_texture() const -> std::shared_ptr<Texture const> { return m_font->get_texture(m_height); }
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
