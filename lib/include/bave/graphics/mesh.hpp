#pragma once
#include <bave/graphics/geometry.hpp>

namespace bave {
class Mesh {
  public:
	struct Data {
		std::span<std::byte const> bytes{};
		std::size_t ibo_offset{};
	};

	[[nodiscard]] auto is_empty() const -> bool { return m_bytes.empty(); }
	[[nodiscard]] auto get_vertex_count() const -> std::uint32_t { return m_verts; }
	[[nodiscard]] auto get_index_count() const -> std::uint32_t { return m_indices; }

	void write(Geometry const& geometry);

	[[nodiscard]] auto get_data() const -> Data { return {.bytes = m_bytes, .ibo_offset = m_ibo_offset}; }

  private:
	std::vector<std::byte> m_bytes{};
	std::uint32_t m_verts{};
	std::uint32_t m_indices{};
	std::size_t m_ibo_offset{};
};
} // namespace bave
