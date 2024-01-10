#include <bave/file_io.hpp>
#include <bave/json_io.hpp>
#include <tools/nine_slicer.hpp>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

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
		auto const uri = truncate_to_uri(path);
		if (uri.empty()) { continue; }
		if (load_uri(uri)) { return; }
	}
}

void NineSlicer::file_menu_items() {
	if (ImGui::MenuItem("New")) { new_slice(); }
	if (ImGui::MenuItem("Open...")) {
		if (auto uri = dialog_open_file("Open"); !uri.empty()) { load_uri(uri); }
	}
	if (ImGui::MenuItem("Save", nullptr, false, !m_json_uri.empty())) { save_slice(); }
	if (ImGui::MenuItem("Save As...", nullptr, false, !m_image_uri.empty())) {
		auto json_uri = fs::path{m_json_uri};
		if (json_uri.empty()) { json_uri = replace_extension(m_image_uri, ".9slice.json"); }
		if (auto uri = dialog_save_file("Save NineSlice", json_uri.generic_string()); !uri.empty()) {
			m_json_uri = uri;
			save_slice();
		}
	}

	ImGui::Separator();
	Applet::file_menu_items();
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

void NineSlicer::metadata_control() {
	auto const size = [&]() -> glm::ivec2 {
		if (auto const& texture = m_image_quad->get_texture()) { return texture->get_size(); }
		return {};
	}();
	image_meta_control(m_image_uri, size);
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
	resize_framebuffer(image_size, {500, 200});

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

	resize_framebuffer(m_image_quad->get_shape().size.current, {500, 200});
	set_title();
}

auto NineSlicer::load_slice(std::string_view const uri) -> bool {
	auto json = m_loader.load_json(uri);
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

void NineSlicer::save_slice() {
	if (m_json_uri.empty()) { return; }
	auto json = dj::Json{};
	json["image"] = m_image_uri;
	to_json(json["nine_slice"], m_image_quad->get_shape().slice);
	if (save_json(json, m_json_uri)) {
		m_log.info("saved NineSlice to '{}'", m_json_uri);
		set_title();
		return;
	}
	m_log.error("failed to save JSON to '{}'", m_json_uri);
}

void NineSlicer::set_title() { get_app().set_title(format_title("NineSlice", m_json_uri, m_unsaved).c_str()); }
} // namespace bave::tools
