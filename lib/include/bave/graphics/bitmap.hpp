#pragma once
#include <bave/core/polymorphic.hpp>
#include <glm/vec2.hpp>
#include <bit>
#include <span>
#include <vector>

namespace bave {
struct BitmapView {
	std::span<std::byte const> bytes{};
	glm::ivec2 extent{};
};

class IBitmapViewSource : public Polymorphic {
  public:
	[[nodiscard]] virtual auto get_bitmap_view() const -> BitmapView = 0;
};

class Bitmap : public IBitmapViewSource {
  public:
	using View = BitmapView;

	explicit Bitmap(std::vector<std::byte> bytes, glm::ivec2 extent) : m_bytes(std::move(bytes)), m_extent(extent) {}

	[[nodiscard]] auto get_bitmap_view() const -> BitmapView final { return {.bytes = std::span<std::byte const>{m_bytes}, .extent = m_extent}; }

  private:
	std::vector<std::byte> m_bytes{};
	glm::ivec2 m_extent{};
};

constexpr auto is_power_of_2(int const in) -> bool { return std::popcount(static_cast<std::uint32_t>(in)) == 1; }
} // namespace bave
