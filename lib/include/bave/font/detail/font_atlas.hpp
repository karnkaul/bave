#pragma once
#include <bave/core/inclusive_range.hpp>
#include <bave/font/detail/glyph_page.hpp>
#include <bave/font/glyph.hpp>
#include <bave/graphics/pixmap.hpp>
#include <bave/graphics/texture.hpp>
#include <memory>

namespace bave::detail {
class FontAtlas {
  public:
	explicit FontAtlas(NotNull<RenderDevice*> render_device, NotNull<GlyphSlot::Factory*> slot_factory, TextHeight height = TextHeight::eDefault);

	[[nodiscard]] auto glyph_for(Codepoint codepoint) const -> Glyph;

	[[nodiscard]] auto get_glyph_page() const -> GlyphPage const& { return m_page; }
	[[nodiscard]] auto get_texture() const -> std::shared_ptr<Texture const> const& { return m_texture; }

  private:
	std::shared_ptr<Texture const> m_texture{};
	GlyphPage m_page;
	std::unordered_map<Codepoint, Glyph> m_glyphs{};
};
} // namespace bave::detail
