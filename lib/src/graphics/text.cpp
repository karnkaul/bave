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

auto Text::get_bounds() const -> Rect<> {
	auto origin = transform.position;
	origin.x += [&] {
		switch (m_align) {
		default:
		case Align::eMid: return 0.0f;
		case Align::eLeft: return -0.5f * m_extent.x;
		case Align::eRight: return 0.5f * m_extent.x;
		}
	}();
	origin.y += 0.5f * m_extent.y;
	return Rect<>::from_extent(m_extent, origin);
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
	m_extent = pen.calc_line_extent(m_text);
	m_x_offset = (n_offset - 0.5f) * m_extent.x;
	pen.cursor.x += m_x_offset;

	set_geometry(pen.generate_quads(m_text));
	set_texture(pen.get_texture());
}
} // namespace bave
