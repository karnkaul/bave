#include <src/font/detail/freetype.hpp>

#if defined(BAVE_USE_FREETYPE)

#include <iostream>

namespace bave::detail {
void FreetypeGlyphFactory::Deleter::operator()(FT_Face face) const noexcept { FT_Done_Face(face); }

FreetypeGlyphFactory::FreetypeGlyphFactory(FT_Face face, std::vector<std::byte> bytes) : m_face(face), m_font_bytes(std::move(bytes)) {}

auto FreetypeGlyphFactory::set_height(TextHeight height) -> bool {
	if (FT_Set_Pixel_Sizes(m_face, 0, static_cast<FT_UInt>(height)) == FT_Err_Ok) {
		m_height = height;
		return true;
	}
	return false;
}

auto FreetypeGlyphFactory::slot_for(Codepoint codepoint) const -> GlyphSlot {
	if (m_face == nullptr) { return {}; }
	if (FT_Load_Char(m_face, static_cast<FT_ULong>(codepoint), FT_LOAD_RENDER) != FT_Err_Ok) { return {}; }
	if (m_face.get()->glyph == nullptr) { return {}; }
	auto ret = GlyphSlot{.codepoint = codepoint};
	auto const& glyph = *m_face.get()->glyph;
	auto const cols = static_cast<int>(glyph.bitmap.width);
	ret.pixmap = Pixmap{glm::ivec2{cols, glyph.bitmap.rows}, blank_v};
	for (int row = 0; row < static_cast<int>(glyph.bitmap.rows); ++row) {
		for (int col = 0; col < static_cast<int>(glyph.bitmap.width); ++col) {
			auto const index = Index2D{col, row};
			ret.pixmap.at(index) = Rgba{.channels = {0xff, 0xff, 0xff, glyph.bitmap.buffer[index.flatten(cols)]}}; // NOLINT
		}
	}
	ret.left_top = {glyph.bitmap_left, glyph.bitmap_top};
	ret.advance = {static_cast<int>(m_face.get()->glyph->advance.x), static_cast<int>(m_face.get()->glyph->advance.y)};
	return ret;
}

void Freetype::Deleter::operator()(FT_Library lib) const noexcept { FT_Done_FreeType(lib); }

Freetype::Freetype() {
	if (FT_Init_FreeType(&m_lib.get()) != FT_Err_Ok) { m_log.error("failed to initialize freetype"); }
}

auto Freetype::load(std::vector<std::byte> bytes) const -> std::unique_ptr<GlyphSlot::Factory> {
	if (m_lib == nullptr) { return {}; }
	auto* face = FT_Face{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	if (FT_New_Memory_Face(m_lib, reinterpret_cast<FT_Byte const*>(bytes.data()), static_cast<FT_Long>(bytes.size()), 0, &face) != FT_Err_Ok) { return {}; }
	return std::make_unique<FreetypeGlyphFactory>(face, std::move(bytes));
}
} // namespace bave::detail

#endif
