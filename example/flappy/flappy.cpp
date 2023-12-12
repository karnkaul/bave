#include <bave/extent_scaler.hpp>
#include <bave/projector.hpp>
#include <flappy.hpp>
#include <cmath>

namespace {
constexpr auto world_space_v = glm::vec2{1440.0f, 2560.0f};
}

Flappy::Flappy(bave::App& app) : Game(app), m_quad(&app.get_render_device()) {
	// m_quad.set_shape(bave::Quad{.size = glm::vec2{300.0f}});

	m_quad.instances = {
		bave::RenderInstance{.transform = bave::Transform{.position = glm::vec2{400.0f}, .rotation = bave::Degrees{15.0f}}, .tint = bave::yellow_v},
	};

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
	auto texture = std::make_shared<bave::Texture>(&app.get_render_device());
	texture->write(bitmap);
	texture->sampler.min = texture->sampler.mag = bave::Sampler::Filter::eNearest;
	m_quad.set_texture(std::move(texture));

	get_app().render_view.viewport = bave::ExtentScaler{.source = get_app().get_framebuffer_size()}.match_width(world_space_v);
}

void Flappy::tick() {
	auto prev_pointer = m_pointer;

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

		if (auto const* tap = std::get_if<bave::MouseClick>(&event)) {
			m_log.info("tap {} at {}x{}", (tap->action == bave::Action::eRelease ? "up" : "down"), tap->position.x, tap->position.y);
			if (tap->id == 0) {
				m_drag = tap->action == bave::Action::ePress;
				prev_pointer = m_pointer = tap->position;
			}
		}

		if (auto const* cursor_move = std::get_if<bave::CursorMove>(&event)) { m_pointer = cursor_move->position; }
	}

	m_elapsed += get_app().get_dt();
	m_clear_red = 0.5f * std::sin(m_elapsed.count()) + 0.5f;
	clear_colour = bave::Rgba::from({m_clear_red, 0.0f, 0.0f, 1.0f});

	m_quad.instances.front().transform.rotation.value += bave::Degrees{get_app().get_dt().count() * 10.0f}.to_radians().value;

	if (m_drag) {
		auto const delta = bave::Projector{.source = get_app().get_framebuffer_size(), .target = world_space_v}(m_pointer - prev_pointer);
		get_app().render_view.transform.position -= delta;
	}

	IFBAVEIMGUI({
		ImGui::ShowDemoWindow();
		// ...
	});
}

void Flappy::render(vk::CommandBuffer command_buffer) const {
	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) {
		m_quad.draw(*shader, command_buffer);
		//
	}
}
