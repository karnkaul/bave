#include <bave/graphics/drawable.hpp>
#include <bave/graphics/shader.hpp>

namespace bave {
Drawable::Drawable(NotNull<RenderDevice*> render_device) : m_mesh(render_device) {}

void Drawable::draw(Shader& shader) const {
	bake_instances();
	update_textures(shader);
	shader.draw(m_mesh, m_baked_instances);
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
