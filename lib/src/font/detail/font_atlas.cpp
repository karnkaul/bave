#include <bave/font/detail/font_atlas.hpp>

namespace bave::detail {
FontAtlas::FontAtlas(NotNull<RenderDevice*> render_device, NotNull<GlyphSlot::Factory*> slot_factory, TextHeight const height) : m_page(slot_factory, height) {
	struct Entry {
		Codepoint codepoint{};
		Glyph glyph{};
	};

	static constexpr int avg_glyphs_per_line = 8;

	auto const max_width = static_cast<int>(height) * avg_glyphs_per_line;
	auto const pad = static_cast<int>(scale_text_height(height, 0.1f));

	auto builder = Pixmap::Builder{max_width, glm::ivec2{pad}};
	auto entries = std::vector<Entry>{};

	auto const add_codepoint = [&](Codepoint const codepoint) {
		auto slot = m_page.slot_for(codepoint);
		if (!slot) { return; }

		auto glyph = Glyph{
			.advance = {slot.advance.x >> 6, slot.advance.y >> 6},
			.extent = slot.pixmap.get_extent(),
			.left_top = slot.left_top,
		};
		auto entry = Entry{.codepoint = codepoint, .glyph = glyph};
		if (slot.has_pixmap()) { builder.add(static_cast<Pixmap::Atlas::Id>(codepoint), std::move(slot.pixmap)); }

		entries.push_back(entry);
	};

	add_codepoint(Codepoint::eTofu);

	for (auto codepoint = Codepoint::eAsciiFirst; codepoint != Codepoint::eAsciiLast; codepoint = static_cast<Codepoint>(static_cast<int>(codepoint) + 1)) {
		add_codepoint(codepoint);
	}

	auto atlas = builder.build(blank_v);
	auto bitmap = atlas.pixmap.make_bitmap();
	auto texture = std::make_shared<Texture>(render_device, bitmap.get_bitmap_view(), true);
	texture->sampler.mag = Texture::Filter::eLinear;
	m_texture = std::move(texture);

	for (auto& entry : entries) {
		entry.glyph.uv_rect = atlas.uvs[static_cast<Pixmap::Atlas::Id>(entry.codepoint)];
		m_glyphs.insert_or_assign(entry.codepoint, entry.glyph);
	}
}

auto FontAtlas::glyph_for(Codepoint codepoint) const -> Glyph {
	if (auto const it = m_glyphs.find(codepoint); it != m_glyphs.end()) { return it->second; }
	return {};
}
} // namespace bave::detail
