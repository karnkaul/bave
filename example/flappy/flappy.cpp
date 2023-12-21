#include <bave/extent_scaler.hpp>
#include <bave/graphics/pixmap.hpp>
#include <bave/graphics/projector.hpp>
#include <flappy.hpp>

namespace {
constexpr auto world_space_v = glm::vec2{1440.0f, 2560.0f};
}

Flappy::Flappy(bave::App& app) : Game(app), m_quad(&app.get_render_device()) {
	// m_quad.set_shape(bave::Quad{.size = glm::vec2{300.0f}});

	m_quad.instances = {
		bave::RenderInstance{.transform = bave::Transform{.position = glm::vec2{400.0f}, .rotation = bave::Degrees{15.0f}}, .tint = bave::yellow_v},
	};

	auto pixels = bave::Pixmap{{2, 2}};
	pixels.at({0, 0}) = bave::red_v;
	pixels.at({0, 1}) = bave::green_v;
	pixels.at({1, 0}) = bave::blue_v;
	pixels.at({1, 1}) = bave::magenta_v;
	auto const bitmap = pixels.make_bitmap();


	if (!m_quad.get_texture()) {
		auto texture = std::make_shared<bave::Texture>(&app.get_render_device());
		texture->write(bitmap.view());
		texture->sampler.min = texture->sampler.mag = bave::Texture::Filter::eNearest;
		m_quad.set_texture(std::move(texture));
	}

	// get_app().get_render_device().render_view.viewport = bave::ExtentScaler{.source = get_app().get_framebuffer_size()}.match_width(world_space_v);
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

		if (auto const* tap = std::get_if<bave::PointerTap>(&event)) {
			auto const& pointer = tap->pointer;
			m_log.info("tap [{}] {} at {}x{}", int(pointer.id), (tap->action == bave::Action::eRelease ? "up" : "down"), pointer.position.x,
					   pointer.position.y);
			if (pointer.id == bave::Pointer::Id::ePrimary) {
				m_drag = tap->action == bave::Action::ePress;
				prev_pointer = m_pointer = tap->pointer.position;
			}
		}

		if (auto const* cursor_move = std::get_if<bave::PointerMove>(&event)) {
			auto const& pointer = cursor_move->pointer;
			if (pointer.id == bave::Pointer::Id::ePrimary) { m_pointer = pointer.position; }
		}
	}

	m_elapsed += get_app().get_dt();
	m_clear_red = 0.5f * std::sin(m_elapsed.count()) + 0.5f;
	clear_colour = bave::Rgba::from({m_clear_red, 0.0f, 0.0f, 1.0f});

	m_quad.instances.front().transform.rotation.value += bave::Degrees{get_app().get_dt().count() * 10.0f}.to_radians().value;

	if (auto const pinch = m_pinch.update(get_app().get_active_pointers())) {
		get_app().get_render_device().render_view.transform.scale += 0.1f * *pinch * get_app().get_dt().count();
		m_drag = false;
	}
	auto const projector = bave::Projector{.source = get_app().get_framebuffer_size(), .target = get_app().get_framebuffer_size()};

	if (m_drag) {
		auto const delta = projector(m_pointer - prev_pointer);
		get_app().get_render_device().render_view.transform.position -= delta;
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
