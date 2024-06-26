#include <bave/io/file_io.hpp>
#include <bave/io/json_io.hpp>
#include <tools/nine_slicer.hpp>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

NineSlicer::NineSlicer(App& app, NotNull<std::shared_ptr<State>> const& state)
	: Applet(app, state), m_loader(&get_app().get_data_store(), &get_app().get_render_device()) {
	m_top.tint = m_bottom.tint = m_left.tint = m_right.tint = red_v;

	load_previous();
}

void NineSlicer::tick() {
	Applet::tick();

	auto nine_quad = m_image_quad.get_shape();
	begin_sidepanel_window("Nine Slice");
	{
		if (ImGui::CollapsingHeader("Nine Slice", ImGuiTreeNodeFlags_DefaultOpen)) { slice_control(nine_quad.slice); }

		if (ImGui::CollapsingHeader("Nine Quad", ImGuiTreeNodeFlags_DefaultOpen)) { ImGui::DragFloat2("size", &nine_quad.size.current.x); }

		if (ImGui::CollapsingHeader("Metadata", ImGuiTreeNodeFlags_DefaultOpen)) { metadata_control(); }

		if (ImGui::CollapsingHeader("Misc")) {
			clear_colour_control();
			zoom_control("Zoom", main_view.scale);
			ImGui::Checkbox("Wireframe", &m_wireframe);
		}
	}
	ImGui::End();

	m_image_quad.set_shape(nine_quad);

	resize_guides();
	position_guides();
}

void NineSlicer::render(Shader& shader) const {
	if (m_wireframe) {
		shader.line_width = 3.0f;
		shader.polygon_mode = vk::PolygonMode::eLine;
	}
	m_image_quad.draw(shader);

	shader.polygon_mode = vk::PolygonMode::eFill;
	m_top.draw(shader);
	m_bottom.draw(shader);
	m_left.draw(shader);
	m_right.draw(shader);
}

void NineSlicer::file_menu_items() {
	if (ImGui::MenuItem("New")) { new_slice(); }
	if (ImGui::MenuItem("Open...")) {
		if (auto uri = dialog_open_file("Open Image / NineSlice"); !uri.empty()) { load_new_uri(uri); }
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

auto NineSlicer::load_new_uri(std::string_view const uri) -> bool {
	if (!load_uri(uri)) {
		m_log.warn("failed to open '{}'", uri);
		return false;
	}

	state->nine_slicer.last_loaded = uri;
	save_state();

	m_log.info("loaded '{}'", uri);
	return true;
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
		if (auto const& texture = m_image_quad.get_texture()) { return texture->get_size(); }
		return {};
	}();
	image_meta_control(m_image_uri, size);
}

void NineSlicer::resize_guides() {
	auto const vert_size = glm::vec2{m_image_quad.get_shape().size.current.x + 100.0f, 3.0f};
	auto const horz_size = glm::vec2{3.0f, m_image_quad.get_shape().size.current.y + 100.0f};
	m_top.set_shape(Quad{.size = vert_size});
	m_bottom.set_shape(Quad{.size = vert_size});
	m_left.set_shape(Quad{.size = horz_size});
	m_right.set_shape(Quad{.size = horz_size});
}

void NineSlicer::position_guides() {
	auto const& nine_slice = m_image_quad.get_shape();
	auto const sized_slice = NineSlice{
		.n_left_top = nine_slice.slice.n_left_top * nine_slice.size.reference,
		.n_right_bottom = (1.0f - nine_slice.slice.n_right_bottom) * nine_slice.size.reference,
	};
	auto const half_size = 0.5f * nine_slice.size.current;
	m_top.transform.position.y = half_size.y - sized_slice.n_left_top.y;
	m_bottom.transform.position.y = -half_size.y + sized_slice.n_right_bottom.y;
	m_left.transform.position.x = half_size.x - sized_slice.n_right_bottom.x;
	m_right.transform.position.x = -half_size.x + sized_slice.n_left_top.x;
}

auto NineSlicer::load_uri(std::string_view const uri) -> bool {
	if (!get_app().get_data_store().exists(uri)) { return false; }
	auto const extension = fs::path{uri}.extension().string();
	if (extension == ".json") { return load_slice(uri); }
	return load_image_at(uri);
}

void NineSlicer::load_previous() {
	if (load_uri(state->nine_slicer.last_loaded)) { return; }
	state->nine_slicer.last_loaded.clear();
	save_state();
	new_slice();
}

auto NineSlicer::load_image_at(std::string_view const uri) -> bool {
	auto texture = m_loader.load_texture(uri);
	if (!texture) { return false; }

	glm::vec2 const image_size = texture->get_size();
	m_image_quad.set_texture(std::move(texture));
	m_image_quad.set_shape(NineQuad{.size = image_size});

	m_image_uri = uri;
	m_json_uri.clear();

	main_view.scale = auto_zoom(image_size);
	set_title();

	m_log.info("loaded '{}'", m_image_uri);
	return true;
}

void NineSlicer::new_slice() {
	m_image_quad.set_texture({});
	m_image_quad.set_shape({});

	m_image_uri.clear();
	m_json_uri.clear();
	m_unsaved = false;

	state->nine_slicer.last_loaded.clear();
	save_state();

	main_view.scale = auto_zoom(m_image_quad.get_shape().size.current);
	set_title();
}

auto NineSlicer::load_slice(std::string_view const uri) -> bool {
	auto json = m_loader.load_json_asset<Texture9Slice>(uri);
	if (!json) { return false; }

	if (!load_image_at(json["image"].as_string())) { return false; }

	auto nine_quad = m_image_quad.get_shape();
	from_json(json["nine_slice"], nine_quad.slice);
	m_image_quad.set_shape(nine_quad);
	m_unsaved = false;

	m_json_uri = uri;
	set_title();

	m_log.info("loaded '{}'", uri);
	return true;
}

void NineSlicer::save_slice() {
	if (m_json_uri.empty()) { return; }
	auto json = dj::Json{};
	json["asset_type"] = get_asset_type<Texture9Slice>();
	json["image"] = m_image_uri;
	to_json(json["nine_slice"], m_image_quad.get_shape().slice);

	if (!save_json(json, m_json_uri)) {
		m_log.error("failed to save NineSlice to '{}'", m_json_uri);
		return;
	}

	m_log.info("saved NineSlice to '{}'", m_json_uri);
	m_unsaved = false;

	state->nine_slicer.last_loaded = m_json_uri;
	save_state();

	set_title();
}

void NineSlicer::set_title() { get_app().set_title(format_title("NineSlicer", m_json_uri, m_unsaved).c_str()); }
} // namespace bave::tools
