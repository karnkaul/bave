#include <bave/core/error.hpp>
#include <bave/font/font.hpp>
#include <bave/graphics/render_device.hpp>
#include <algorithm>

namespace bave {
Font::Font(NotNull<RenderDevice*> render_device) : m_render_device(render_device) {}

auto Font::load_from_bytes(std::vector<std::byte> file_bytes, float scale) -> bool {
	auto slot_factory = m_render_device->get_font_library().load(std::move(file_bytes));
	if (!slot_factory) { return false; }
	m_slot_factory = std::move(slot_factory);
	m_scale = std::clamp(scale, scale_limit_v.lo, scale_limit_v.hi);
	return true;
}

auto Font::glyph_for(TextHeight height, Codepoint codepoint) -> Glyph {
	if (auto const* atlas = get_font_atlas(height)) {
		auto ret = atlas->glyph_for(codepoint);
		ret.scale(1.0f / m_scale);
		return ret;
	}
	return {};
}
auto Font::get_texture(TextHeight height) -> std::shared_ptr<Texture const> {
	if (auto const* atlas = get_font_atlas(height)) { return atlas->get_texture(); }
	return {};
}

auto Font::get_font_atlas(TextHeight height) -> Ptr<detail::FontAtlas const> {
	height = clamp_text_height(height);
	if (auto it = m_atlases.find(height); it != m_atlases.end()) { return &it->second; }

	if (!is_loaded()) { return {}; }

	auto const scaled_height = scale_text_height(height, m_scale);
	auto [it, _] = m_atlases.insert_or_assign(height, detail::FontAtlas{m_render_device, m_slot_factory.get(), scaled_height});
	return &it->second;
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
