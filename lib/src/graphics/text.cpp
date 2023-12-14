#include <bave/graphics/text.hpp>

namespace bave {
Text::Text(NotNull<RenderDevice*> render_device, std::shared_ptr<Font> font) : Drawable(render_device), m_font(std::move(font)) {}

auto Text::set_font(std::shared_ptr<Font> font) -> Text& {
	if (m_font.get() != font.get()) {
		m_font = std::move(font);
		refresh();
	}
	return *this;
}

auto Text::set_string(std::string text) -> Text& {
	if (text != m_text) {
		m_text = std::move(text);
		refresh();
	}
	return *this;
}

auto Text::set_height(Height height) -> Text& {
	height = clamp_text_height(height);
	if (height != m_height) {
		m_height = height;
		refresh();
	}
	return *this;
}

auto Text::set_align(Align const align) -> Text& {
	if (align != m_align) {
		m_align = align;
		refresh();
	}
	return *this;
}

auto Text::set_scale(float scale) -> Text& {
	if (scale >= 0.0f && scale != m_scale) {
		m_scale = scale;
		refresh();
	}
	return *this;
}

void Text::refresh() {
	if (m_text.empty() || m_scale == 0.0f || !m_font) {
		set_geometry({});
		return;
	}

	auto const n_offset = [&] {
		switch (m_align) {
		// NOLINTNEXTLINE
		case Align::eLeft: return -0.5f;
		// NOLINTNEXTLINE
		case Align::eRight: return +0.5f;
		default: return 0.0f;
		}
	}();

	auto pen = Font::Pen{m_font.get(), m_height, m_scale};
	pen.cursor += (n_offset - 0.5f) * pen.calc_line_extent(m_text).x;

	set_geometry(pen.generate_quads(m_text));
	set_texture(pen.get_texture());
}
} // namespace bave
