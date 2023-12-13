#include <bave/core/error.hpp>
#include <bave/graphics/pixmap.hpp>
#include <utility>

namespace bave {
namespace {
constexpr auto ceil_pot(int const size) -> int {
	auto ret = 1;
	while (ret < size) { ret <<= 1; }
	return ret;
}

constexpr auto ceil_pot(glm::ivec2 const size) -> glm::ivec2 { return {ceil_pot(size.x), ceil_pot(size.y)}; }
} // namespace

Pixmap::Pixmap(glm::ivec2 const size, Rgba const background) : m_size(size), m_pixels(static_cast<std::size_t>(m_size.x * m_size.y), background) {}

auto Pixmap::overwrite(Pixmap const& source, Index2D const left_top) -> bool {
	auto const rb = Index2D{left_top.x + source.m_size.x, left_top.y + source.m_size.y};
	if (rb.x > m_size.x || rb.y > m_size.y) { return false; }

	for (int row = 0; row < source.m_size.y; ++row) {
		for (int col = 0; col < source.m_size.x; ++col) { at({col + left_top.x, row + left_top.y}) = source.at({col, row}); }
	}
	return true;
}

auto Pixmap::at(Index2D index) const -> Rgba const& {
	auto const idx = static_cast<std::size_t>(index.flatten(m_size.x));
	if (idx >= m_pixels.size()) { throw Error{"Out of bounds index: '{}x{}'", index.x, index.y}; }
	return m_pixels.at(idx);
}

auto Pixmap::at(Index2D index) -> Rgba& {
	return const_cast<Rgba&>(std::as_const(*this).at(index)); // NOLINT(cppcoreguidelines-pro-type-const-cast)
}

auto Pixmap::make_bitmap() const -> Bitmap {
	auto ret = Bitmap{.extent = m_size};
	ret.bytes.reserve(m_pixels.size() * 4);
	for (auto const& pixel : m_pixels) {
		auto const bytes = pixel.to_bytes();
		ret.bytes.insert(ret.bytes.end(), bytes.begin(), bytes.end());
	}
	return ret;
}

Pixmap::Builder::Builder(int max_width, glm::ivec2 pad) : m_max_width(max_width > min_width_v ? ceil_pot(max_width) : min_width_v), m_pad(pad) {
	m_data.cursor = m_pad;
}

void Pixmap::Builder::add(Id id, Pixmap pixmap) {
	if (pixmap.m_pixels.empty()) { return; }

	if (m_data.cursor.x + pixmap.m_size.x + m_pad.x >= m_max_width) { line_break(); }

	auto const lt = m_data.cursor;
	auto const rb = lt + pixmap.m_size;
	m_data.line_height = std::max(m_data.line_height, pixmap.m_size.y);
	m_entries.push_back(Entry{.pixmap = std::move(pixmap), .rect = {.lt = lt, .rb = rb}, .id = id});
	m_data.cursor.x += pixmap.m_size.x + m_pad.x;
	m_data.size.x = std::max(m_data.size.x, m_data.cursor.x);
	m_data.size.y = std::max(m_data.size.y, m_data.current_height + m_data.line_height + m_pad.y);
}

auto Pixmap::Builder::build(Rgba const background) const -> Atlas {
	if (m_entries.empty()) { return {}; }
	auto ret = Atlas{};
	ret.pixmap = Pixmap{ceil_pot(m_data.size), background};
	auto const fsize = glm::vec2{ret.pixmap.m_size};
	for (auto const& entry : m_entries) {
		ret.pixmap.overwrite(entry.pixmap, {entry.rect.lt.x, entry.rect.lt.y});
		auto const fu = glm::vec2{entry.rect.top_left()} / fsize;
		auto const fv = glm::vec2{entry.rect.bottom_right()} / fsize;
		ret.uvs.insert_or_assign(entry.id, UvRect{.lt = fu, .rb = fv});
	}
	return ret;
}

void Pixmap::Builder::line_break() {
	m_data.cursor.x = m_pad.x;
	m_data.cursor.y += m_data.line_height + 2 * m_pad.y;
	m_data.current_height = m_data.cursor.y;
}
} // namespace bave
