#include <tools/applet.hpp>
#include <tools/nine_slicer.hpp>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

Applet::Applet(App& app, NotNull<std::shared_ptr<State>> const& state) : m_app(&app), state(state) {
	app.set_title("Bave Tools");
	main_view.position.x = -150.0f;
}

void Applet::render() const {
	auto render_view = get_app().get_render_device().get_default_view();
	render_view.transform = main_view;
	get_app().get_render_device().render_view = render_view;

	if (auto shader = get_app().load_shader("shaders/default.vert", "shaders/default.frag")) { render(*shader); }
}

void Applet::on_key(KeyInput const& key_input) {
	if (key_input.key == Key::eLeftControl || key_input.key == Key::eRightControl) { m_ctrl_pressed = key_input.action != Action::eRelease; }
}

void Applet::on_scroll(MouseScroll const& scroll) {
	if (!m_ctrl_pressed) { return; }
	change_zoom(scroll.delta.y * zoom_scroll_rate, get_app().get_active_pointers().front().position);
}

void Applet::on_drop(std::span<std::string const> paths) {
	for (auto const& path : paths) {
		auto const uri = truncate_to_uri(path);
		if (uri.empty()) { continue; }
		if (load_new_uri(uri)) { return; }
	}
}

void Applet::file_menu_items() {
	if (ImGui::MenuItem("Change mount point...")) {
		auto result = pfd::select_folder("Select mount point").result();
		if (!result.empty()) { get_app().change_mount_point(result); }
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Quit")) { get_app().shutdown(); }
}

void Applet::save_state() {
	if (state->save()) {
		m_log.info("State saved to '{}'", State::path_v.as_view());
	} else {
		m_log.warn("failed to save State to '{}'", State::path_v.as_view());
	}
}

void Applet::change_zoom(float delta, glm::vec2 /*cursor_position*/) { main_view.scale = zoom_scale_range_v.clamp(main_view.scale + delta); }

auto Applet::auto_zoom(glm::vec2 const content_area, glm::vec2 const pad) const -> glm::vec2 {
	auto const total_area = content_area + pad;
	if (!is_positive(total_area)) { return glm::vec2{1.0f}; }
	auto const scaled_content_area = ExtentScaler{.source = total_area}.fit_space(get_app().get_framebuffer_size());
	return glm::vec2{scaled_content_area.x / total_area.x};
}

void Applet::begin_lt_window(CString label, bool resizeable) {
	ImGui::SetNextWindowPos({0.0f, y_top_v}, ImGuiCond_Always);
	auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	if (!resizeable) { flags |= ImGuiWindowFlags_NoResize; }
	ImGui::Begin(label.c_str(), nullptr, flags);
}

void Applet::begin_fullscreen_window(CString const label) const {
	glm::vec2 const fb_size = get_app().get_framebuffer_size();
	ImGui::SetNextWindowSize({fb_size.x, fb_size.y - y_top_v}, ImGuiCond_Always);
	begin_lt_window(label, false);
}

void Applet::begin_sidepanel_window(CString const label, float const min_width) {
	glm::vec2 const fb_size = get_app().get_framebuffer_size();
	ImGui::SetNextWindowSizeConstraints({min_width, fb_size.y - y_top_v}, {fb_size.x, fb_size.y - y_top_v});
	begin_lt_window(label, true);
	m_sidepanel_width = ImGui::GetWindowWidth();
}

auto Applet::drag_ivec2(CString label, glm::ivec2& out, InclusiveRange<glm::ivec2> range, float width) -> bool {
	ImGui::SetNextItemWidth(width);
	auto ret = ImGui::DragInt(FixedString{"##{}", label.as_view()}.c_str(), &out.x, 1.0f, range.lo.x, range.hi.x);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(width);
	ret |= ImGui::DragInt(label.c_str(), &out.y, 1.0f, range.lo.y, range.hi.y);
	return ret;
}

auto Applet::drag_irect(Rect<int>& out, InclusiveRange<Rect<int>> range, bool positional) -> bool {
	auto ret = false;
	ret |= ImGui::DragInt("left", &out.lt.x, 1.0f, range.lo.lt.x, range.hi.lt.x);
	ret |= ImGui::DragInt("top", &out.lt.y, 1.0f, range.lo.lt.y, range.hi.lt.y);
	ret |= ImGui::DragInt("right", &out.rb.x, 1.0f, range.lo.rb.x, range.hi.rb.x);
	ret |= ImGui::DragInt("bottom", &out.rb.y, 1.0f, range.lo.rb.y, range.hi.rb.y);
	if (positional) {
		auto const rect_size = out.rb - out.lt;
		if (drag_ivec2("position", out.lt, {.hi = range.hi.rb - rect_size})) {
			out.rb = out.lt + rect_size;
			ret = true;
		}
	}
	return ret;
}

void Applet::zoom_control(CString label, glm::vec2& out_scale) {
	static constexpr auto zoom_range_f = InclusiveRange<glm::vec2>{.lo = zoom_scale_range_v.lo * 100.0f, .hi = zoom_scale_range_v.hi * 100.0f};
	static constexpr auto zoom_range_i = InclusiveRange<int>{static_cast<int>(zoom_range_f.lo.x), static_cast<int>(zoom_range_f.hi.x)};
	auto i_zoom = static_cast<int>(out_scale.x * 100.0f);
	if (ImGui::SliderInt(label.c_str(), &i_zoom, zoom_range_i.lo, zoom_range_i.hi)) { out_scale = glm::vec2{static_cast<float>(i_zoom) / 100.0f}; }
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset")) { out_scale = glm::vec2{1.0f}; }
}

void Applet::clear_colour_control() {
	auto rgba = clear_colour.to_vec4();
	if (ImGui::ColorEdit3("clear colour", &rgba.x)) { clear_colour = Rgba::from(rgba); }
}

void Applet::image_meta_control(std::string_view const image_uri, glm::ivec2 const size) {
	im_text("Image: {}", or_none(image_uri).data());
	im_text("Size: {} x {}", size.x, size.y);
	auto const pot_x = is_power_of_2(size.x);
	auto const pot_y = is_power_of_2(size.y);
	im_text("POT: {} ({} x {})", (pot_x && pot_y), pot_x, pot_y);
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

auto Applet::dialog_open_file(CString const title) const -> std::string {
	auto const mount_point = get_app().get_data_store().get_mount_point();
	auto result = pfd::open_file(title.c_str(), fs::path{mount_point}.make_preferred().string()).result();
	if (result.empty()) { return {}; }
	return truncate_to_uri(result.front());
}

auto Applet::dialog_save_file(CString const title, std::string_view const uri) const -> std::string {
	auto const mount_point = fs::path{get_app().get_data_store().get_mount_point()};
	auto const path = (mount_point / uri).make_preferred().string();
	return truncate_to_uri(pfd::save_file(title.c_str(), path).result());
}

auto Applet::save_json(dj::Json const& json, std::string_view uri) const -> bool {
	auto const mount_point = fs::path{get_app().get_data_store().get_mount_point()};
	auto const path = (mount_point / uri).make_preferred().string();
	return json.to_file(path.c_str());
}

auto Applet::format_title(std::string_view name, std::string_view uri, bool unsaved) -> std::string {
	auto ret = fmt::format("{}{}", unsaved ? "*" : "", name);
	if (!uri.empty()) { fmt::format_to(std::back_inserter(ret), " - {}", uri); }
	return ret;
}
} // namespace bave::tools
