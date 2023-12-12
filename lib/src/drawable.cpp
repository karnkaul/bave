#include <bave/drawable.hpp>
#include <bave/graphics/shader.hpp>

namespace bave {
Drawable::Drawable(NotNull<App*> app) : m_app(app), m_mesh(&app->get_render_device()) {}

void Drawable::draw(vk::CommandBuffer command_buffer) const {
	if (!command_buffer) { return; }

	auto shader = m_app->load_shader(vertex_shader, fragment_shader);
	if (!shader) { return; }

	bake_instances();
	update_textures(*shader);
	shader->draw(command_buffer, m_mesh, m_baked_instances);
}

void Drawable::bake_instances() const {
	m_baked_instances.clear();
	m_baked_instances.reserve(instances.size() + 1);
	m_baked_instances.push_back(RenderInstance{.transform = transform, .tint = tint}.bake());
	for (auto const& instance : instances) { m_baked_instances.push_back(instance.bake()); }
}

void Drawable::update_textures(Shader& out_shader) const {
	auto image_samplers = std::array<ImageSampler, SetLayout::max_textures_v>{};
	for (std::uint32_t binding = 0; binding < textures.size(); ++binding) {
		auto const& texture = textures.at(binding);
		if (texture == nullptr) { continue; }
		image_samplers.at(binding) = texture->combined_image_sampler();
	}
	out_shader.update_textures(image_samplers);
}
} // namespace bave
