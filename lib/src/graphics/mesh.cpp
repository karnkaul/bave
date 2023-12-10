#include <bave/graphics/mesh.hpp>

namespace bave {
Mesh::Mesh(NotNull<RenderDevice*> render_device)
	: m_render_device(render_device), m_vbo(&render_device->get_defer_queue(), render_device->get_vertex_buffer_cache().allocate()) {}

void Mesh::write(Geometry const& geometry) {
	if (geometry.vertices.empty()) {
		m_data.clear();
		m_verts = m_indices = 0;
		return;
	}

	m_ibo_offset = geometry.vertices.size() * sizeof(geometry.vertices[0]);
	auto const ibo_size = geometry.indices.size() * sizeof(geometry.indices[0]);
	m_data.resize(m_ibo_offset + ibo_size);
	std::memcpy(m_data.data(), geometry.vertices.data(), m_ibo_offset);
	if (ibo_size > 0) {
		std::memcpy(m_data.data() + m_ibo_offset, geometry.indices.data(), ibo_size); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	}

	m_verts = static_cast<std::uint32_t>(geometry.vertices.size());
	m_indices = static_cast<std::uint32_t>(geometry.indices.size());
}

void Mesh::draw(vk::CommandBuffer command_buffer, std::uint32_t instance_count) const {
	auto* vbo = get_buffer();
	if (vbo == nullptr) { return; }

	command_buffer.bindVertexBuffers(0, vbo->get_buffer(), vk::DeviceSize{});
	if (m_indices > 0) {
		command_buffer.bindIndexBuffer(vbo->get_buffer(), m_ibo_offset, vk::IndexType::eUint32);
		command_buffer.drawIndexed(m_indices, instance_count, 0, 0, 0);
	} else {
		command_buffer.draw(m_verts, instance_count, 0, 0);
	}
}

auto Mesh::get_buffer() const -> Ptr<RenderBuffer> {
	if (m_data.empty()) { return {}; }

	auto const& ret = m_vbo.get().at(m_render_device->get_frame_index());
	ret->write(m_data.data(), m_data.size());
	return ret.get();
}
} // namespace bave
