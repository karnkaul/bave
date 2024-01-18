#include <bave/graphics/drawable.hpp>
#include <bave/graphics/shader.hpp>

namespace bave {
namespace {
[[nodiscard]] auto make_bounds(std::span<bave::Vertex const> vertices, bave::Transform const& transform) {
	auto const matrix = transform.matrix();
	auto ret = Rect<>{};
	ret.lt.x = ret.rb.y = std::numeric_limits<float>::max();
	ret.lt.y = ret.rb.x = -std::numeric_limits<float>::max();
	for (auto const& vertex : vertices) {
		auto const position = glm::vec2{matrix * glm::vec4{vertex.position, 0.0f, 1.0f}};
		ret.lt.x = std::min(ret.lt.x, position.x);
		ret.lt.y = std::max(ret.lt.y, position.y);
		ret.rb.y = std::min(ret.rb.y, position.y);
		ret.rb.x = std::max(ret.rb.x, position.x);
	}
	return ret;
}
} // namespace

Drawable::Drawable(NotNull<RenderDevice*> render_device) : m_mesh(render_device) {}

void Drawable::draw(Shader& shader) const {
	bake_instances();
	update_textures(shader);
	shader.draw(m_mesh, m_baked_instances);
}

auto Drawable::get_bounds() const -> Rect<> { return make_bounds(m_geometry.vertices, transform); }

void Drawable::set_geometry(Geometry geometry) {
	m_geometry = std::move(geometry);
	m_mesh.write(m_geometry);
}

void Drawable::bake_instances() const {
	m_baked_instances.clear();
	if (instances.empty()) {
		m_baked_instances.push_back(RenderInstance{.transform = transform, .tint = tint}.bake());
	} else {
		m_baked_instances.reserve(instances.size());
		for (auto const& instance : instances) { m_baked_instances.push_back(instance.bake()); }
	}
}

void Drawable::update_textures(Shader& out_shader) const {
	auto image_samplers = std::array<CombinedImageSampler, SetLayout::max_textures_v>{};
	for (std::uint32_t binding = 0; binding < textures.size(); ++binding) {
		auto const& texture = textures.at(binding);
		if (texture == nullptr) { continue; }
		image_samplers.at(binding) = texture->combined_image_sampler();
	}
	out_shader.update_textures(image_samplers);
}
} // namespace bave
