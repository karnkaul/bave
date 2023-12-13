#pragma once
#include <bave/core/index_2d.hpp>
#include <bave/graphics/bitmap.hpp>
#include <bave/graphics/rect.hpp>
#include <bave/graphics/rgba.hpp>
#include <unordered_map>
#include <vector>

namespace bave {
class Pixmap {
  public:
	struct Atlas;
	class Builder;

	explicit Pixmap(glm::ivec2 size = {1, 1}, Rgba background = black_v);

	[[nodiscard]] auto get_size() const -> glm::ivec2 { return m_size; }

	auto overwrite(Pixmap const& source, Index2D left_top) -> bool;

	[[nodiscard]] auto at(Index2D index) const -> Rgba const&;
	[[nodiscard]] auto at(Index2D index) -> Rgba&;

	[[nodiscard]] auto make_bitmap() const -> Bitmap;

  private:
	glm::ivec2 m_size{};
	std::vector<Rgba> m_pixels{};
};

struct Pixmap::Atlas {
	using Id = std::uint32_t;

	Pixmap pixmap{};
	std::unordered_map<Id, UvRect> uvs{};
};

class Pixmap::Builder {
  public:
	using Id = Atlas::Id;

	explicit Builder(int max_cols, glm::ivec2 pad = {1, 1});

	void add(Id id, Pixmap pixmap);

	[[nodiscard]] auto build(Rgba background = black_v) const -> Atlas;

  private:
	void line_break();

	struct Entry {
		Pixmap pixmap{};
		Rect2D<int> rect{};
		Id id{};
	};

	std::vector<Entry> m_entries{};
	int m_max_cols;
	glm::ivec2 m_pad;

	struct {
		int current_height{};
		int line_height{};
		glm::ivec2 cursor{};
		int col{};
		glm::ivec2 size{};
	} m_data{};
};
} // namespace bave
