#include <bave/font/detail/glyph_page.hpp>

namespace bave::detail {
GlyphPage::GlyphPage(NotNull<GlyphSlot::Factory*> slot_factory, TextHeight height) : m_slot_factory(slot_factory), m_height(height) {}

auto GlyphPage::slot_for(Codepoint const codepoint) -> GlyphSlot {
	auto const it = m_slots.find(codepoint);
	if (it != m_slots.end()) { return it->second; }

	auto ret = m_slot_factory->slot_for(codepoint, m_height);
	if (!ret) { return {}; }

	return m_slots.insert_or_assign(codepoint, ret).first->second;
}
} // namespace bave::detail
