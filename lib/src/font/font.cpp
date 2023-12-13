#include <bave/core/error.hpp>
#include <bave/font/font.hpp>
#include <bave/graphics/render_device.hpp>

namespace bave {
auto Font::try_make(NotNull<RenderDevice*> render_device, std::vector<std::byte> file_bytes, float scale) -> std::optional<Font> {
	auto ret = std::optional<Font>{};
	auto slot_factory = render_device->get_font_library().load(std::move(file_bytes));
	if (!slot_factory) { return ret; }
	ret.emplace(render_device, std::move(slot_factory), scale);
	return ret;
}

Font::Font(NotNull<RenderDevice*> render_device, std::unique_ptr<GlyphSlot::Factory> slot_factory, float scale)
	: m_render_device(render_device), m_slot_factory(std::move(slot_factory)), m_scale(scale) {
	if (!m_slot_factory) { throw Error{"Null GlyphSlot::Factory"}; }
}

auto Font::get_font_atlas(TextHeight height) -> FontAtlas& {
	height = clamp_text_height(height);
	if (auto it = m_atlases.find(height); it != m_atlases.end()) { return it->second; }

	auto const create_info = FontAtlas::CreateInfo{.height = scale_text_height(height, m_scale)};
	auto [it, _] = m_atlases.insert_or_assign(height, FontAtlas{m_render_device, m_slot_factory.get(), create_info});
	return it->second;
}

struct Font::Pen::Writer {
	Font::Pen const& pen; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	template <typename Func>
	void operator()(std::string_view const line, Func func) const {
		for (char const ch : line) {
			if (ch == '\n') { return; }
			auto glyph = pen.m_font->glyph_for(pen.m_height, static_cast<Codepoint>(ch));
			if (!glyph) { glyph = pen.m_font->glyph_for(pen.m_height, Codepoint::eTofu); }
			if (!glyph) { continue; }
			func(glyph);
		}
	}
};

auto Font::Pen::advance(std::string_view line) -> Pen& {
	Writer{*this}(line, [this](Glyph const& glyph) { cursor += m_scale * glm::vec2{glyph.advance}; });
	return *this;
}

auto Font::Pen::generate_quads(std::string_view line) -> Geometry {
	auto ret = Geometry{};
	generate_quads(ret, line);
	return ret;
}

auto Font::Pen::generate_quads(Geometry& out, std::string_view line) -> Pen& {
	Writer{*this}(line, [this, &out](Glyph const& glyph) {
		auto const rect = glyph.rect(cursor, m_scale);
		auto const quad = Quad{
			.size = rect.extent(),
			.uv = glyph.uv_rect,
			.rgba = vertex_colour,
			.origin = rect.centre(),
		};
		out.append(quad);
		cursor += m_scale * glm::vec2{glyph.advance};
	});
	return *this;
}

auto Font::Pen::calc_line_extent(std::string_view line) const -> glm::vec2 {
	auto ret = glm::vec2{};
	Writer{*this}(line, [&ret](Glyph const& glyph) {
		ret.x += glyph.advance.x;
		ret.y = std::max(ret.y, glyph.extent.y);
	});
	return ret;
}
} // namespace bave
