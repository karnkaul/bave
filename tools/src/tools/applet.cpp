#include <tools/applet.hpp>

namespace bave::tools {
auto Applet::make_bootloader(State const& state) -> std::function<std::unique_ptr<Applet>(App&)> {
	return [state](App& app) {
		if (auto const it = s_applets.find(state.active_applet); it != s_applets.end()) { return it->second(app, state); }
		return std::make_unique<Applet>(app, state);
	};
}

Applet::Applet(App& app, State state) : Game(app), state(std::move(state)) { app.set_title(title_prefix_v.data()); }

void Applet::tick() { main_menu_bar(); }

void Applet::render() const {
	auto render_view = get_app().get_render_device().get_default_view();
	render_view.transform.position = view_position;
	render_view.transform.scale = glm::vec2{static_cast<float>(zoom) / 100.0f};
	get_app().get_render_device().render_view = render_view;

	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) {
		if (wireframe) {
			shader->polygon_mode = vk::PolygonMode::eLine;
			shader->line_width = 3.0f;
		}
		for (auto const& drawable : drawables) { drawable->draw(*shader); }
	}
}

void Applet::on_key(KeyInput const& key_input) {
	if (key_input.action == Action::ePress && key_input.key == Key::eW && key_input.mods == make_key_mods(mod::ctrl)) { get_app().shutdown(); }
}

void Applet::on_scroll(MouseScroll const& scroll) {
	auto f_range = static_cast<InclusiveRange<float>>(zoom_range_v);
	zoom = std::clamp(zoom + scroll.delta.y * zoom_scroll_rate, f_range.lo, f_range.hi);
}

void Applet::zoom_control() {
	auto i_zoom = static_cast<int>(zoom);
	if (ImGui::SliderInt("Zoom", &i_zoom, zoom_range_v.lo, zoom_range_v.hi)) { zoom = static_cast<float>(i_zoom); }
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset")) { zoom = 100.0f; }
}

void Applet::wireframe_control() { ImGui::Checkbox("Wireframe", &wireframe); }

void Applet::file_menu_items() {
	if (ImGui::MenuItem("Change mount point...")) {
		auto result = pfd::select_folder("Select mount point").result();
		if (!result.empty()) { get_app().change_mount_point(result); }
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Quit")) { get_app().shutdown(); }
}

void Applet::save_state() {
	if (state.save()) {
		m_log.info("State saved to '{}'", State::path_v.as_view());
	} else {
		m_log.warn("failed to save State to '{}'", State::path_v.as_view());
	}
}

void Applet::begin_lt_window(CString name, bool resizeable) {
	ImGui::SetNextWindowPos({0.0f, y_top_v}, ImGuiCond_Always);
	auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	if (!resizeable) { flags |= ImGuiWindowFlags_NoResize; }
	ImGui::Begin(name.c_str(), nullptr, flags);
}

void Applet::begin_fullscreen_window(CString const name) const {
	glm::vec2 const fb_size = get_app().get_framebuffer_size();
	ImGui::SetNextWindowSize({fb_size.x, fb_size.y - y_top_v}, ImGuiCond_Always);
	begin_lt_window(name, false);
}

void Applet::begin_sidepanel_window(CString const name, float const min_width) {
	glm::vec2 const fb_size = get_app().get_framebuffer_size();
	ImGui::SetNextWindowSizeConstraints({min_width, fb_size.y - y_top_v}, {fb_size.x, fb_size.y - y_top_v});
	begin_lt_window(name, true);
	m_sidepanel_width = ImGui::GetWindowWidth();
}

void Applet::main_menu_bar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			file_menu_items();
			ImGui::EndMenu();
		}

		main_menu();
		applet_menu();

		ImGui::EndMainMenuBar();
	}
}

void Applet::applet_menu() {
	if (ImGui::BeginMenu("Applets")) {
		for (auto const& [name, factory] : s_applets) {
			if (ImGui::MenuItem(name.data())) {
				m_log.info("loading '{}'", name);
				state.active_applet = name;
				save_state();
				replace_next_frame(factory(get_app(), std::move(state)));
			}
		}
		ImGui::EndMenu();
	}
}
} // namespace bave::tools
