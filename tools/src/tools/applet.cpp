#include <tools/applet.hpp>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

auto Applet::make_bootloader(State const& state) -> std::function<std::unique_ptr<Applet>(App&)> {
	return [state](App& app) {
		if (auto const it = s_applets.find(state.active_applet); it != s_applets.end()) { return it->second(app, state); }
		return std::make_unique<Applet>(app, state);
	};
}

auto Applet::drag_ivec2(CString label, glm::ivec2& out, InclusiveRange<glm::ivec2> range, float width) -> bool {
	ImGui::SetNextItemWidth(width);
	auto ret = ImGui::DragInt(FixedString{"##{}", label.as_view()}.c_str(), &out.x, 1.0f, range.lo.x, range.hi.x);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(width);
	ret |= ImGui::DragInt(label.c_str(), &out.y, 1.0f, range.lo.y, range.hi.y);
	return ret;
}

Applet::Applet(App& app, State state) : Game(app), state(std::move(state)) { app.set_title(title_prefix_v.data()); }

void Applet::tick() { main_menu_bar(); }

void Applet::render() const {
	auto render_view = get_app().get_render_device().get_default_view();
	render_view.transform.position = view_position;
	render_view.transform.scale = glm::vec2{static_cast<float>(zoom) / 100.0f};
	get_app().get_render_device().render_view = render_view;

	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) { render(*shader); }
}

void Applet::on_key(KeyInput const& key_input) {
	if (key_input.action == Action::ePress && key_input.key == Key::eW && key_input.mods == make_key_mods(mod::ctrl)) { get_app().shutdown(); }
}

void Applet::on_scroll(MouseScroll const& scroll) {
	auto f_range = static_cast<InclusiveRange<float>>(zoom_range_v);
	zoom = std::clamp(zoom + scroll.delta.y * zoom_scroll_rate, f_range.lo, f_range.hi);
}

void Applet::render(Shader& shader) const {
	if (wireframe) {
		shader.polygon_mode = vk::PolygonMode::eLine;
		shader.line_width = 3.0f;
	}
	for (auto const& drawable : drawables) { drawable->draw(shader); }
}

void Applet::zoom_control() {
	auto i_zoom = static_cast<int>(zoom);
	if (ImGui::SliderInt("Zoom", &i_zoom, zoom_range_v.lo, zoom_range_v.hi)) { zoom = static_cast<float>(i_zoom); }
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset")) { zoom = 100.0f; }
}

void Applet::wireframe_control() { ImGui::Checkbox("Wireframe", &wireframe); }

void Applet::image_meta_control(std::string_view const image_uri, glm::ivec2 const size) {
	im_text("Image: {}", or_none(image_uri).data());
	im_text("Size: {} x {}", size.x, size.y);
	auto const pot_x = is_power_of_2(size.x);
	auto const pot_y = is_power_of_2(size.y);
	im_text("POT: {} ({} x {})", (pot_x && pot_y), pot_x, pot_y);
}

void Applet::file_menu_items() {
	if (ImGui::MenuItem("Change mount point...")) {
		auto result = pfd::select_folder("Select mount point").result();
		if (!result.empty()) { get_app().change_mount_point(result); }
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Quit")) { get_app().shutdown(); }
}

auto Applet::replace_extension(std::string_view uri, std::string_view extension) -> std::string {
	auto ret = fs::path{uri};
	ret.replace_extension(extension);
	return ret.generic_string();
}

auto Applet::truncate_to_uri(std::string_view const path) const -> std::string {
	auto uri = get_app().get_data_store().make_uri(path);
	if (uri.empty() || uri.starts_with("..")) {
		m_log.error("path located outside mount point: '{}'", path);
		return {};
	}
	return uri;
}

void Applet::resize_framebuffer(glm::ivec2 const size, glm::ivec2 const pad) { get_app().set_framebuffer_size(size + pad); }

auto Applet::dialog_open_file(CString const title) const -> std::string {
	auto const mount_point = get_app().get_data_store().get_mount_point();
	auto result = pfd::open_file(title.c_str(), mount_point.data()).result();
	if (result.empty()) { return {}; }
	return truncate_to_uri(result.front());
}

auto Applet::dialog_save_file(CString const title, std::string_view const uri) const -> std::string {
	auto const mount_point = get_app().get_data_store().get_mount_point();
	auto const path = (fs::path{mount_point} / uri).generic_string();
	return truncate_to_uri(pfd::save_file(title.c_str(), path).result());
}

auto Applet::save_json(dj::Json const& json, std::string_view uri) const -> bool {
	auto const mount_point = fs::path{get_app().get_data_store().get_mount_point()};
	auto const path = (mount_point / uri).generic_string();
	return json.to_file(path.c_str());
}

auto Applet::format_title(std::string_view name, std::string_view uri, bool unsaved) -> std::string {
	auto ret = fmt::format("{}{}", unsaved ? "*" : "", name);
	if (!uri.empty()) { fmt::format_to(std::back_inserter(ret), " - {}", uri); }
	return ret;
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
