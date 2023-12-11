#include <flappy.hpp>
#include <cmath>

Flappy::Flappy(bave::App& app) : Game(app), m_mesh(&app.get_render_device()), m_texture(&app.get_render_device()) {
	m_mesh.write(bave::Geometry::from(bave::Quad{.size = glm::vec2{300.0f}}));

	auto pixels = std::array<std::uint32_t, 4>{
		0xff0000ff,
		0xff00ff00,
		0xffff0000,
		0xffff00ff,
	};
	auto const bitmap = bave::Bitmap{
		.bytes = {reinterpret_cast<std::byte const*>(pixels.data()), pixels.size() * sizeof(pixels[0])},
		.extent = {2, 2},
	};
	m_texture.write(bitmap);
	m_texture.sampler.min = m_texture.sampler.mag = bave::Sampler::Filter::eNearest;
}

void Flappy::tick() {
	for (auto const& event : get_app().get_events()) {
		if (auto const* focus_change = std::get_if<bave::FocusChange>(&event)) {
			m_log.info("focus {}", focus_change->in_focus ? "gained" : "lost");
			//
		}

		if (auto const* key_input = std::get_if<bave::KeyInput>(&event)) {
			if (key_input->key == bave::Key::eW && key_input->mods.test(bave::mod::ctrl)) {
				m_log.info("shutting down");
				get_app().shutdown();
			}

			if (key_input->key == bave::Key::eEscape && key_input->action == bave::Action::eRelease) {
				m_log.info("shutting down");
				get_app().shutdown();
			}
		}

		if (auto const* mouse_click = std::get_if<bave::MouseClick>(&event)) {
			m_log.info("tap {} at {}x{}", (mouse_click->action == bave::Action::eRelease ? "up" : "down"), mouse_click->position.x, mouse_click->position.y);
		}
	}

	m_elapsed += get_app().get_dt();
	m_clear_red = 0.5f * std::sin(m_elapsed.count()) + 0.5f;
	clear_colour = bave::Rgba::from({m_clear_red, 0.0f, 0.0f, 1.0f});

	IFBAVEIMGUI({
		ImGui::ShowDemoWindow();
		// ...
	});
}

void Flappy::render(vk::CommandBuffer command_buffer) const {
	auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag");
	if (shader) {
		shader->update(1, 0, m_texture.combined_image_sampler());
		auto const instances = std::array{
			bave::RenderInstance::Baked{.transform = glm::mat4{1.0f}, .rgba = glm::vec4{1.0f}},
			bave::RenderInstance{.transform = bave::Transform{.position = glm::vec2{400.0f}}, .rgba = bave::yellow_v}.bake(),
		};
		shader->draw(command_buffer, m_mesh, instances);
	}
}
