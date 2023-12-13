#pragma once
#include <bave/core/not_null.hpp>
#include <bave/font/detail/glyph_slot.hpp>
#include <unordered_map>

namespace bave::detail {
class GlyphPage {
  public:
	explicit GlyphPage(NotNull<GlyphSlot::Factory*> slot_factory, TextHeight height = TextHeight::eDefault);

	[[nodiscard]] auto slot_for(Codepoint codepoint) -> GlyphSlot;

	[[nodiscard]] auto get_slot_factory() const -> GlyphSlot::Factory& { return *m_slot_factory; }
	[[nodiscard]] auto get_text_height() const -> TextHeight { return m_height; }
	[[nodiscard]] auto get_slot_count() const -> std::size_t { return m_slots.size(); }

  private:
	std::unordered_map<Codepoint, GlyphSlot> m_slots{};
	NotNull<GlyphSlot::Factory*> m_slot_factory;
	TextHeight m_height{};
};
} // namespace bave::detail
