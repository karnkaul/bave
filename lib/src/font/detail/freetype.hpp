#pragma once
#include <bave/core/scoped_resource.hpp>
#include <bave/font/font_library.hpp>
#include <bave/logger.hpp>

#if defined(BAVE_USE_FREETYPE)

#include <ft2build.h>
#include FT_FREETYPE_H

namespace bave::detail {
class FreetypeGlyphFactory : public GlyphSlot::Factory {
  public:
	explicit FreetypeGlyphFactory(FT_Face face, std::vector<std::byte> bytes);

	auto set_height(TextHeight height) -> bool final;
	[[nodiscard]] auto height() const -> TextHeight final { return m_height; }
	[[nodiscard]] auto slot_for(Codepoint codepoint) const -> GlyphSlot final;

  private:
	struct Deleter {
		void operator()(FT_Face face) const noexcept;
	};

	ScopedResource<FT_Face, Deleter> m_face{};
	std::vector<std::byte> m_font_bytes{};
	TextHeight m_height{};
};

class Freetype : public FontLibrary {
  public:
	explicit Freetype();

	[[nodiscard]] auto load(std::vector<std::byte> bytes) const -> std::unique_ptr<GlyphSlot::Factory> final;

  private:
	struct Deleter {
		void operator()(FT_Library lib) const noexcept;
	};

	Logger m_log{"Freetype"};
	ScopedResource<FT_Library, Deleter> m_lib{};
};
} // namespace bave::detail

#endif
