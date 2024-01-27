#pragma once
#include <bave/core/index_2d.hpp>
#include <bave/graphics/bitmap.hpp>
#include <bave/graphics/rect.hpp>
#include <bave/graphics/rgba.hpp>
#include <unordered_map>
#include <vector>

namespace bave {
/// \brief Writeable array of Rgba.
class Pixmap {
  public:
	struct Atlas;
	class Builder;

	/// \brief Constructor.
	/// \param extent Extent (size) of bitmap.
	/// \param background initializer for all Rgba values.
	explicit Pixmap(glm::ivec2 extent = {1, 1}, Rgba background = black_v);

	[[nodiscard]] auto get_extent() const -> glm::ivec2 { return m_extent; }

	/// \brief Overwrite image data.
	/// \param source Source Pixmap to read from.
	/// \param left_top Destination offset to start overwriting at.
	/// \returns true on success.
	auto overwrite(Pixmap const& source, Index2D left_top) -> bool;

	/// \brief Obtain Rgba at given Index2D.
	/// \returns Const reference to Rgba.
	[[nodiscard]] auto at(Index2D index) const -> Rgba const&;
	/// \brief Obtain Rgba at given Index2D.
	/// \returns Mutable reference to Rgba.
	[[nodiscard]] auto at(Index2D index) -> Rgba&;

	/// \brief Create a byte bitmap from stored Rgba bitmap.
	/// \returns Rgba Bitmap.
	[[nodiscard]] auto make_bitmap() const -> Bitmap;

	[[nodiscard]] auto is_empty() const -> bool { return m_pixels.empty(); }

  private:
	glm::ivec2 m_extent{};
	std::vector<Rgba> m_pixels{};
};

/// \brief Atlas of sub-images.
struct Pixmap::Atlas {
	using Id = std::uint32_t;

	/// \brief Combined Pixmap.
	Pixmap pixmap{};
	/// \brief UV coordinates of each sub-image.
	std::unordered_map<Id, UvRect> uvs{};
};

/// \brief Builder for Pixmap::Atlas.
class Pixmap::Builder {
  public:
	using Id = Atlas::Id;

	static constexpr int min_width_v{16};

	/// \brief Constructor.
	/// \param max_width Maximum width of final pixmap.
	/// \param pad Padding between each sub-image.
	explicit Builder(int max_width, glm::ivec2 pad = {1, 1});

	/// \brief Add a sub-image.
	/// \param id Id to associate with.
	/// \param pixmap Sub-image to add.
	void add(Id id, Pixmap pixmap);

	/// \brief Buld Atlas.
	/// \param background Background Rgba.
	/// \returns Built Atlas.
	[[nodiscard]] auto build(Rgba background = black_v) const -> Atlas;

  private:
	void line_break();

	struct Entry {
		Pixmap pixmap{};
		Rect<int> rect{};
		Id id{};
	};

	std::vector<Entry> m_entries{};
	int m_max_width;
	glm::ivec2 m_pad;

	struct {
		glm::ivec2 cursor{};
		int current_height{};
		int line_height{};
		glm::ivec2 size{};
	} m_data{};
};
} // namespace bave
