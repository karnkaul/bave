#include <bave/graphics/drawable.hpp>
#include <bave/graphics/shader.hpp>

namespace bave {
void Drawable::Primitive::write(Geometry const& geometry) {
	if (geometry.vertex_array.is_empty()) {
		clear();
		return;
	}

	auto const vs = std::span{geometry.vertex_array.vertices};
	auto const is = std::span{geometry.vertex_array.indices};
	ibo_offset = vs.size_bytes();
	auto const ibo_size = is.size_bytes();
	bytes.resize(ibo_offset + ibo_size);
	std::memcpy(bytes.data(), vs.data(), vs.size_bytes());
	if (ibo_size > 0) {
		std::memcpy(bytes.data() + ibo_offset, is.data(), is.size_bytes()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	}

	verts = static_cast<std::uint32_t>(vs.size());
	indices = static_cast<std::uint32_t>(is.size());
	topology = geometry.topology;
}

void Drawable::Primitive::clear() {
	bytes.clear();
	ibo_offset = 0;
	verts = indices = 0;
	topology = Topology::eTriangleList;
}

Drawable::Primitive::operator RenderPrimitive() const {
	return RenderPrimitive{.bytes = bytes, .ibo_offset = ibo_offset, .vertices = verts, .indices = indices, .topology = topology};
}

void Drawable::draw(Shader& shader) const {
	auto const baked_instance = to_baked();
	update_textures(shader);
	shader.draw(m_primitive, {&baked_instance, 1});
}

void Drawable::set_geometry(Geometry geometry) {
	m_geometry = std::move(geometry);
	m_primitive.write(m_geometry);
}

void Drawable::update_textures(Shader& out_shader) const {
	auto image_samplers = std::array<SamplerImage, Shader::max_textures_v>{};
	for (std::uint32_t binding = 0; binding < textures.size(); ++binding) {
		auto const& texture = textures.at(binding);
		if (texture == nullptr) { continue; }
		image_samplers.at(binding) = texture->get_sampler_image();
	}
	out_shader.update_textures(image_samplers);
}
} // namespace bave
