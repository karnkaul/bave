#include <bave/graphics/mesh.hpp>

namespace bave {
void Mesh::write(Geometry const& geometry) {
	if (geometry.vertices.empty()) {
		m_bytes.clear();
		m_verts = m_indices = 0;
		return;
	}

	m_ibo_offset = geometry.vertices.size() * sizeof(geometry.vertices[0]);
	auto const ibo_size = geometry.indices.size() * sizeof(geometry.indices[0]);
	m_bytes.resize(m_ibo_offset + ibo_size);
	std::memcpy(m_bytes.data(), geometry.vertices.data(), m_ibo_offset);
	if (ibo_size > 0) {
		std::memcpy(m_bytes.data() + m_ibo_offset, geometry.indices.data(), ibo_size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	}

	m_verts = static_cast<std::uint32_t>(geometry.vertices.size());
	m_indices = static_cast<std::uint32_t>(geometry.indices.size());
}
} // namespace bave
