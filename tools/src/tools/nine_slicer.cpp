#include <bave/core/fixed_string.hpp>
#include <bave/file_io.hpp>
#include <bave/json_io.hpp>
#include <tools/nine_slicer.hpp>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

namespace {
[[nodiscard]] constexpr auto or_none(std::string_view const in) -> std::string_view {
	if (in.empty()) { return "[none]"; }
	return in;
}
} // namespace

NineSlicer::NineSlicer(App& app, State state) : Applet(app, std::move(state)), m_loader(&get_app().get_data_store(), &get_app().get_render_device()) {
	m_image_quad = push(std::make_unique<NineQuadShape>(&app.get_render_device()));
	m_top = push(std::make_unique<QuadShape>(&app.get_render_device()));
	m_bottom = push(std::make_unique<QuadShape>(&app.get_render_device()));
	m_left = push(std::make_unique<QuadShape>(&app.get_render_device()));
	m_right = push(std::make_unique<QuadShape>(&app.get_render_device()));

	m_top->tint = m_bottom->tint = m_left->tint = m_right->tint = red_v;

	new_slice();

	view_position.x = -150.0f;
}

void NineSlicer::tick() {
	Applet::tick();

	auto nine_quad = m_image_quad->get_shape();
	begin_sidepanel_window("Nine Slice");
	{
		if (ImGui::CollapsingHeader("Nine Slice", ImGuiTreeNodeFlags_DefaultOpen)) { slice_control(nine_quad.slice); }

		if (ImGui::CollapsingHeader("Nine Quad", ImGuiTreeNodeFlags_DefaultOpen)) { ImGui::DragFloat2("size", &nine_quad.size.current.x); }

		if (ImGui::CollapsingHeader("Metadata", ImGuiTreeNodeFlags_DefaultOpen)) { metadata_control(); }

		if (ImGui::CollapsingHeader("Misc")) {
			zoom_control();
			wireframe_control();
		}
	}
	ImGui::End();

	m_image_quad->set_shape(nine_quad);

	resize_guides();
	position_guides();
}

void NineSlicer::on_drop(std::span<std::string const> paths) {
	for (auto const& path : paths) {
		if (try_load_path(path)) { return; }
	}
}

void NineSlicer::file_menu_items() {
	auto const mount_point = get_app().get_data_store().get_mount_point();
	if (ImGui::MenuItem("New")) { new_slice(); }
	if (ImGui::MenuItem("Open...")) {
		auto result = pfd::open_file("Open", mount_point.data()).result();
		if (!result.empty()) { try_load_path(result.front()); }
	}
	if (ImGui::MenuItem("Save", nullptr, false, !m_json_uri.empty())) { save_slice(); }
	if (ImGui::MenuItem("Save As...", nullptr, false, !m_image_uri.empty())) {
		auto json_uri = fs::path{m_json_uri};
		if (json_uri.empty()) { json_uri = make_json_uri(); }
		auto const path = (fs::path{mount_point} / json_uri).generic_string();
		auto result = pfd::save_file("Save NineSlice", path).result();
		if (!result.empty()) { try_save_slice(result); }
	}

	ImGui::Separator();
	Applet::file_menu_items();
}

void NineSlicer::metadata_control() {
	ImGui::Text("Image: %s", or_none(m_image_uri).data());
	auto const size = [&]() -> glm::ivec2 {
		auto const& texture = m_image_quad->get_texture();
		if (!texture) { return {}; }
		return texture->get_size();
	}();
	ImGui::Text("%s", FixedString{"Size: {} x {}", size.x, size.y}.c_str());
	auto const pot_x = is_power_of_2(size.x);
	auto const pot_y = is_power_of_2(size.y);
	ImGui::Text("%s", FixedString{"POT: {} ({} x {})", (pot_x && pot_y), pot_x, pot_y}.c_str());
}

void NineSlicer::slice_control(NineSlice& out) {
	auto changed = ImGui::DragFloat("left", &out.n_left_top.x, 0.002f, 0.0f, 1.0f);
	changed |= ImGui::DragFloat("top", &out.n_left_top.y, 0.002f, 0.0f, 1.0f);
	changed |= ImGui::DragFloat("right", &out.n_right_bottom.x, 0.002f, 0.0f, 1.0f);
	changed |= ImGui::DragFloat("bottom", &out.n_right_bottom.y, 0.002f, 0.0f, 1.0f);
	if (ImGui::Button("Reset")) {
		out = NineSlice{};
		changed = true;
	}
	if (changed && !m_unsaved && !m_json_uri.empty()) {
		m_unsaved = true;
		set_title();
	}
}

void NineSlicer::resize_guides() {
	auto const vert_size = glm::vec2{m_image_quad->get_shape().size.current.x + 100.0f, 3.0f};
	auto const horz_size = glm::vec2{3.0f, m_image_quad->get_shape().size.current.y + 100.0f};
	m_top->set_shape(Quad{.size = vert_size});
	m_bottom->set_shape(Quad{.size = vert_size});
	m_left->set_shape(Quad{.size = horz_size});
	m_right->set_shape(Quad{.size = horz_size});
}

void NineSlicer::position_guides() {
	auto const& nine_slice = m_image_quad->get_shape();
	auto const sized_slice = NineSlice{
		.n_left_top = nine_slice.slice.n_left_top * nine_slice.size.reference,
		.n_right_bottom = (1.0f - nine_slice.slice.n_right_bottom) * nine_slice.size.reference,
	};
	auto const half_size = 0.5f * nine_slice.size.current;
	m_top->transform.position.y = half_size.y - sized_slice.n_left_top.y;
	m_bottom->transform.position.y = -half_size.y + sized_slice.n_right_bottom.y;
	m_left->transform.position.x = half_size.x - sized_slice.n_right_bottom.x;
	m_right->transform.position.x = -half_size.x + sized_slice.n_left_top.x;
}

void NineSlicer::resize_framebuffer() {
	glm::vec2 const image_size = m_image_quad->get_shape().size.current;
	auto const fb_size = image_size + glm::vec2{500.0f, 200.0f};
	get_app().set_framebuffer_size(fb_size);
}

auto NineSlicer::try_load_path(std::string_view const path) -> bool {
	auto const uri = try_make_uri(path);
	if (uri.empty()) { return false; }

	if (load_uri(uri)) { return true; }

	return false;
}

auto NineSlicer::load_uri(std::string_view const uri) -> bool {
	auto const extension = fs::path{uri}.extension().string();

	if (extension == ".json") { return load_slice(uri); }
	if (load_image_at(uri)) { return true; }

	m_log.warn("failed to open '{}'", uri);
	return false;
}

auto NineSlicer::load_image_at(std::string_view const uri) -> bool {
	auto texture = m_loader.load_texture(uri);
	if (!texture) { return false; }

	glm::vec2 const image_size = texture->get_size();
	m_image_quad->set_texture(std::move(texture));
	m_image_quad->set_shape(NineQuad{.size = image_size});
	resize_framebuffer();

	m_image_uri = uri;

	m_log.info("loaded '{}'", m_image_uri);

	return true;
}

void NineSlicer::new_slice() {
	m_image_quad->set_texture({});
	m_image_quad->set_shape({});
	m_image_uri.clear();
	m_json_uri.clear();
	m_unsaved = false;

	resize_framebuffer();
	set_title();
}

auto NineSlicer::load_slice(std::string_view const uri) -> bool {
	auto json = dj::Json::from_file(get_app().get_data_store().make_full_path(uri).c_str());
	if (!json || !json.contains("image") || !json.contains("nine_slice")) { return false; }

	if (!load_image_at(json["image"].as_string())) { return false; }

	auto nine_quad = m_image_quad->get_shape();
	from_json(json["nine_slice"], nine_quad.slice);
	m_image_quad->set_shape(nine_quad);
	m_unsaved = false;

	m_json_uri = uri;
	set_title();

	return true;
}

auto NineSlicer::try_save_slice(std::string_view const path) -> bool {
	auto const uri = try_make_uri(path);
	if (uri.empty()) { return false; }

	m_json_uri = uri;
	save_slice();
	return true;
}

void NineSlicer::save_slice() {
	if (m_json_uri.empty()) { return; }
	auto json = dj::Json{};
	json["image"] = m_image_uri;
	to_json(json["nine_slice"], m_image_quad->get_shape().slice);
	auto const path = (fs::path{get_app().get_data_store().get_mount_point()} / m_json_uri).generic_string();
	if (json.to_file(path.c_str())) {
		m_log.info("saved NineSlice to '{}'", m_json_uri);
		set_title();
		return;
	}
	m_log.error("failed to save JSON to '{}'", path);
}

void NineSlicer::set_title() {
	auto title = std::format("{}NineSlicer", m_unsaved ? "*" : "");
	if (!m_json_uri.empty()) { fmt::format_to(std::back_inserter(title), " - {}", m_json_uri); }
	get_app().set_title(title.c_str());
}

auto NineSlicer::try_make_uri(std::string_view const path) const -> std::string {
	auto uri = get_app().get_data_store().make_uri(path);
	if (uri.empty() || uri.starts_with("..")) {
		m_log.error("path located outside mount point: '{}'", path);
		return {};
	}
	return uri;
}

auto NineSlicer::make_json_uri() const -> std::string {
	auto ret = fs::path{m_image_uri};
	ret = ret.parent_path() / ret.stem();
	ret += ".9slice.json";
	return ret.generic_string();
}
} // namespace bave::tools
