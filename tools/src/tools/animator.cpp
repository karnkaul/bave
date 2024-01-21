#include <bave/graphics/extent_scaler.hpp>
#include <tools/animator.hpp>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

Animator::Animator(App& app, NotNull<std::shared_ptr<State>> const& state) : Applet(app, state), m_loader(&app.get_data_store(), &app.get_render_device()) {
	setup_scene();
	if (!load_previous()) { new_timeline(); }
}

void Animator::tick() {
	Applet::tick();

	begin_sidepanel_window("Sprite Animation");

	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) { timeline_control(); }

	if (ImGui::CollapsingHeader("Metadata", ImGuiTreeNodeFlags_DefaultOpen)) { metadata_control(); }

	if (ImGui::CollapsingHeader("Misc")) { misc_control(); }

	ImGui::End();

	m_sprite.tick(get_app().get_dt());

	if (!m_texture) { return; }
	auto const tex_size = m_texture->get_size();
	if (auto const tile = m_texture->find_tile(m_sprite.get_current_tile_id())) {
		auto const size_ratio = m_image_quad.get_shape().size / glm::vec2{tex_size};

		auto outline = LineRect{};
		outline.size = glm::vec2{tile->size} * size_ratio;
		m_rect.set_geometry(Geometry::from(outline));

		auto const top_left = (glm::vec2{glm::ivec2{tile->top_left.x, -tile->top_left.y} + glm::ivec2{-tex_size.x / 2, tex_size.y / 2}}) * size_ratio;
		auto const centre = top_left + 0.5f * glm::vec2{outline.size.x, -outline.size.y};
		m_rect.transform.position = m_image_quad.transform.position + centre;
	}
}

void Animator::render(Shader& shader) const {
	auto const top_scissor = Rect<>{.lt = {}, .rb = {1.0f, 0.5f}};
	auto const bottom_scissor = Rect<>{.lt = {0.0f, 0.5f}, .rb = glm::vec2{1.0f}};

	auto& render_view = get_app().get_render_device().render_view;

	render_view.transform = m_top_view;
	render_view.n_scissor = top_scissor;
	m_image_quad.draw(shader);

	render_view.transform = m_bottom_view;
	render_view.n_scissor = bottom_scissor;
	m_sprite.draw(shader);

	render_view.transform = main_view;
	render_view.n_scissor = uv_rect_v;
	m_separator.draw(shader);

	render_view.transform = m_top_view;
	render_view.n_scissor = top_scissor;
	shader.polygon_mode = vk::PolygonMode::eLine;
	shader.topology = vk::PrimitiveTopology::eLineStrip;
	shader.line_width = 2.0f;
	m_rect.draw(shader);
}

void Animator::change_zoom(float const delta, glm::vec2 const cursor_position) {
	auto& target = cursor_position.y > 0.0f ? m_top_view.scale : m_bottom_view.scale;
	target = zoom_scale_range_v.clamp(target + delta);
}

auto Animator::load_new_uri(std::string_view const uri) -> bool {
	auto const extension = fs::path{uri}.extension().string();
	if (extension != ".json") {
		m_log.error("unrecognized extension: '{}'", uri);
		return false;
	}

	if (can_load_anim() && load_timeline(uri)) {
		m_log.info("loaded SpriteAnimation: '{}'", uri);
		state->animator.last_timeline = uri;
		save_state();
		return true;
	}

	if (load_atlas(uri)) {
		m_log.info("loaded TextureAtlas: '{}'", uri);
		state->animator.last_atlas = uri;
		state->animator.last_timeline.clear();
		save_state();
		return true;
	}

	m_log.warn("failed to open '{}'", uri);
	return false;
}

void Animator::file_menu_items() {
	if (ImGui::MenuItem("New")) { new_timeline(); }
	if (ImGui::BeginMenu("Open")) {
		if (ImGui::MenuItem("Atlas...")) {
			if (auto uri = dialog_open_file("Open Atlas"); !uri.empty()) { load_new_atlas(uri); }
		}
		if (ImGui::MenuItem("Animation...", nullptr, false, can_load_anim())) {
			if (auto uri = dialog_open_file("Open Animation"); !uri.empty()) { load_new_timeline(uri); }
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save", nullptr, false, !state->animator.last_timeline.empty())) { save_timeline(); }
	if (ImGui::MenuItem("Save As...", nullptr, false, !state->animator.last_atlas.empty())) {
		auto json_uri = fs::path{state->animator.last_timeline};
		if (json_uri.empty()) { json_uri = replace_extension(state->animator.last_atlas, ".anim.json"); }
		if (auto uri = dialog_save_file("Save Animation", json_uri.generic_string()); !uri.empty()) {
			state->animator.last_timeline = uri;
			save_timeline();
		}
	}

	ImGui::Separator();
	Applet::file_menu_items();
}

void Animator::timeline_control() {
	auto duration = m_timeline->duration.count();

	ImGui::BeginDisabled(!m_texture);
	ImGui::Checkbox("animate", &m_sprite.animate);
	ImGui::EndDisabled();
	auto const ratio = duration > 0.0f ? m_sprite.elapsed.count() / duration : 0.0f;
	ImGui::ProgressBar(ratio, ImVec2{-FLT_MIN, 0.0f}, FixedString{"{:.1f}", m_sprite.elapsed.count()}.c_str());

	auto modified = false;
	if (ImGui::DragFloat("duration", &duration, 0.05f, 0.0f, 500.0f)) {
		m_timeline->duration = Seconds{duration};
		modified = true;
	}

	if (ImGui::Button("Push")) {
		m_timeline->tiles.push_back(get_next_tile_id());
		modified = true;
	}
	if (ImGui::TreeNode("tiles")) {
		modified |= tiles_control();
		ImGui::TreePop();
	}
	if (!m_timeline->tiles.empty() && ImGui::Button("Pop")) {
		m_timeline->tiles.pop_back();
		modified = true;
	}

	if (modified) {
		m_sprite.set_timeline(m_timeline);
		if (!m_unsaved && !state->animator.last_timeline.empty()) {
			m_unsaved = true;
			set_title();
		}
	}
}

auto Animator::get_next_tile_id() const -> std::string {
	if (m_tile_ids.empty()) { return std::string{"?"}; }
	if (m_timeline->tiles.empty()) { return m_tile_ids.front(); }
	auto const& previous = m_timeline->tiles.back();
	auto const it = std::find(m_tile_ids.begin(), m_tile_ids.end(), previous);
	if (it == m_tile_ids.end()) { return m_tile_ids.front(); }
	auto const index = static_cast<std::size_t>(it - m_tile_ids.begin());
	return m_tile_ids[(index + 1) % m_tile_ids.size()];
}

auto Animator::tiles_control() -> bool {
	auto ret = false;
	for (std::size_t i = 0; i < m_timeline->tiles.size(); ++i) {
		auto& tile_id = m_timeline->tiles.at(i);
		if (ImGui::BeginCombo(FixedString{"[{}]", i}.c_str(), tile_id.c_str())) {
			for (auto const& id : m_tile_ids) {
				if (ImGui::Selectable(id.c_str(), id == tile_id)) {
					tile_id = id;
					ret = true;
				}
			}
			ImGui::EndCombo();
		}
	}
	return ret;
}

void Animator::metadata_control() { im_text<128>("Atlas: {}", or_none(state->animator.last_atlas)); }

void Animator::misc_control() {
	im_text("Zoom");
	zoom_control("Top", m_top_view.scale);
	zoom_control("Bottom", m_bottom_view.scale);
}

auto Animator::load_previous() -> bool {
	auto ret = false;

	if (!state->animator.last_atlas.empty() && !(ret = load_atlas(state->animator.last_atlas))) {
		state->animator.last_atlas.clear();
		state->animator.last_timeline.clear();
		ret = false;
	}

	if (ret && !state->animator.last_timeline.empty() && !(ret = load_timeline(state->animator.last_timeline))) {
		state->animator.last_timeline.clear();
		ret = false;
	}

	if (!ret) {
		save_state();
		return false;
	}

	return true;
}

auto Animator::load_atlas(std::string_view uri) -> bool {
	if (auto texture = m_loader.load_texture_atlas(uri)) {
		m_texture = std::move(texture);
		m_image_quad.set_texture(m_texture);

		auto quad = m_image_quad.get_shape();
		quad.size = m_texture->get_size();
		quad.size = ExtentScaler{m_texture->get_size()}.fit_space(quad_size_v);
		m_image_quad.set_shape(quad);

		m_tile_ids.clear();
		m_tile_ids.reserve(m_texture->get_blocks().size());
		for (auto const& block : m_texture->get_blocks()) { m_tile_ids.push_back(block.id); }
		m_sprite.set_texture_atlas(m_texture);
		m_sprite.animate = true;

		m_unsaved = false;

		m_log.info("opened TiledTexture: '{}'", uri);
		return true;
	}

	m_log.error("failed to open TiledTexture: '{}'", uri);
	return false;
}

void Animator::load_new_atlas(std::string_view uri) {
	if (!load_atlas(uri)) { return; }

	new_timeline();
	state->animator.last_timeline.clear();
	state->animator.last_atlas = uri;
	save_state();
}

void Animator::new_timeline() {
	m_timeline->tiles.clear();
	m_timeline->duration = 1s;
	m_sprite.set_timeline(m_timeline);
	m_sprite.set_uv(uv_rect_v);

	m_rect.set_geometry({});

	state->animator.last_timeline.clear();
	m_unsaved = false;

	set_title();
}

auto Animator::load_timeline(std::string_view uri) -> bool {
	auto timeline = m_loader.load_anim_timeline(uri);
	if (!timeline) { return false; }

	m_timeline = std::move(timeline);
	m_sprite.set_timeline(m_timeline);
	m_unsaved = false;

	set_title();

	return true;
}

void Animator::load_new_timeline(std::string_view const uri) {
	if (!load_timeline(uri)) { return; }

	state->animator.last_timeline = uri;
	save_state();
}

void Animator::save_timeline() {
	if (state->animator.last_timeline.empty()) { return; }
	auto json = dj::Json{};
	json["asset_type"] = get_asset_type<AnimTimeline>();
	json["duration"] = m_timeline->duration.count();
	if (!m_timeline->tiles.empty()) {
		auto& out_tile_ids = json["tiles"];
		for (auto const& in_tile : m_timeline->tiles) { out_tile_ids.push_back(in_tile); }
	}

	if (!save_json(json, state->animator.last_timeline)) {
		m_log.error("failed to save SpriteAnimation to '{}'", state->animator.last_timeline);
		return;
	}

	m_log.info("saved SpriteAnimation to '{}'", state->animator.last_timeline);
	m_unsaved = false;
	set_title();
}

void Animator::setup_scene() {
	glm::vec2 const fb_size = get_app().get_framebuffer_size();

	m_separator.set_shape(Quad{.size = glm::vec2{fb_size.x, 5.0f}});
	m_separator.transform.position.x = main_view.position.x;
	m_image_quad.set_shape(Quad{.size = quad_size_v});
	m_sprite.set_auto_size(quad_size_v);

	m_bottom_view.position.y = 0.25f * fb_size.y;
	m_top_view.position.y = -m_bottom_view.position.y;
	m_top_view.position.x = m_bottom_view.position.x = main_view.position.x;

	m_rect.tint = red_v;

	main_view.position.x = -150.0f;
}

void Animator::set_title() { get_app().set_title(format_title("Animator", state->animator.last_timeline, m_unsaved).c_str()); }
} // namespace bave::tools
