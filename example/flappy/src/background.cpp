#include <bave/core/random.hpp>
#include <src/background.hpp>

using bave::NotNull;
using bave::random_in_range;
using bave::Seconds;
using bave::Shader;

Background::Background(NotNull<Config const*> config) : m_config(config), m_top(config->background_rgba_top), m_bottom(config->background_rgba_bottom) {
	create_quad();
	create_clouds();
}

void Background::tick(Seconds const dt) {
	for (auto& cloud_instance : m_cloud_instances) {
		cloud_instance.position.x -= cloud_instance.speed * dt.count();
		if (cloud_instance.position.x + 0.5f * m_config->cloud_size.x < -0.5f * m_config->world_space.x) { cloud_instance = make_cloud(true); }
	}

	cloud.instances.resize(m_cloud_instances.size());
	for (std::size_t i = 0; i < m_cloud_instances.size(); ++i) { cloud.instances.at(i).transform.position = m_cloud_instances.at(i).position; }
}

void Background::draw(Shader& shader) const {
	quad.draw(shader);
	cloud.draw(shader);
}

void Background::create_quad() {
	m_top = m_config->background_rgba_top;
	m_bottom = m_config->background_rgba_bottom;
	auto geometry = bave::Quad{.size = m_config->world_space}.to_geometry();
	geometry.vertices[0].rgba = geometry.vertices[1].rgba = bave::Rgba::to_linear(m_top.to_vec4());
	geometry.vertices[2].rgba = geometry.vertices[3].rgba = bave::Rgba::to_linear(m_bottom.to_vec4());
	quad.set_geometry(geometry);
}

auto Background::make_cloud(bool beyond_edge) const -> CloudInstance {
	auto ret = CloudInstance{.speed = random_in_range(m_config->cloud_speed_min, m_config->cloud_speed_max)};
	ret.position.y = 0.5f * (random_in_range(-m_config->world_space.y, m_config->world_space.y) - m_config->cloud_size.y);
	if (beyond_edge) {
		ret.position.x = 0.5f * (m_config->world_space.x + m_config->cloud_size.x);
	} else {
		ret.position.x = 0.5f * random_in_range(-m_config->world_space.x, m_config->world_space.x);
	}
	return ret;
}

void Background::create_clouds() {
	cloud.set_size(m_config->cloud_size);
	cloud.set_texture(m_config->cloud_texture);
	m_cloud_instances.reserve(static_cast<std::size_t>(m_config->cloud_instances));
	for (int i = 0; i < m_config->cloud_instances; ++i) { m_cloud_instances.push_back(make_cloud(false)); }
}
