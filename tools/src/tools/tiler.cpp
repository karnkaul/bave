#include <bave/json_io.hpp>
#include <tools/tiler.hpp>
#include <algorithm>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

Tiler::Tiler(App& app, NotNull<std::shared_ptr<State>> const& state)
	: Applet(app, state), m_loader(&get_app().get_data_store(), &get_app().get_render_device()) {
	m_sprite = push(std::make_unique<Sprite>());

	load_previous();
}

void Tiler::tick() {
	Applet::tick();

	begin_sidepanel_window("Sprite Sheet");

	if (ImGui::CollapsingHeader("Tiles", ImGuiTreeNodeFlags_DefaultOpen)) { tiles_control(); }

	if (ImGui::CollapsingHeader("Metadata", ImGuiTreeNodeFlags_DefaultOpen)) { metadata_control(); }

	if (ImGui::CollapsingHeader("Misc")) { zoom_control("Zoom", main_view.scale); }

	ImGui::End();
}

void Tiler::render(Shader& shader) const {
	for (auto const& drawable : drawables) { drawable->draw(shader); }

	shader.polygon_mode = vk::PolygonMode::eLine;
	shader.topology = vk::PrimitiveTopology::eLineStrip;
	shader.line_width = m_outline_width;

	for (auto const& block : m_blocks) { block.rect.draw(shader); }
}

void Tiler::file_menu_items() {
	if (ImGui::MenuItem("New")) { new_atlas(); }
	if (ImGui::MenuItem("Open...")) {
		if (auto uri = dialog_open_file("Open Image / TextureAtlas"); !uri.empty()) { load_new_uri(uri); }
	}
	if (ImGui::MenuItem("Save", nullptr, false, !m_json_uri.empty())) { save_atlas(); }
	if (ImGui::MenuItem("Save As...", nullptr, false, !m_image_uri.empty())) {
		auto json_uri = fs::path{m_json_uri};
		if (json_uri.empty()) { json_uri = replace_extension(m_image_uri, ".atlas.json"); }
		if (auto uri = dialog_save_file("Save TextureAtlas", json_uri.generic_string()); !uri.empty()) {
			m_json_uri = uri;
			save_atlas();
		}
	}

	ImGui::Separator();
	Applet::file_menu_items();
}

auto Tiler::load_new_uri(std::string_view const uri) -> bool {
	if (!load_uri(uri)) {
		m_log.warn("failed to open '{}'", uri);
		return false;
	}

	state->tiler.last_loaded = uri;
	save_state();

	m_log.info("loaded '{}'", uri);
	return true;
}

void Tiler::tiles_control() {
	auto rgba = m_block_rgba.to_vec4();
	if (ImGui::ColorEdit3("RGB", &rgba.x)) { m_block_rgba = Rgba::from(rgba); }
	if (ImGui::Button("Add Block")) { m_blocks.push_back(make_block(static_cast<int>(m_blocks.size()))); }

	ImGui::Separator();
	glm::ivec2 const size = m_sprite->get_size();
	auto const origin_offset = 0.5f * glm::vec2{-size.x, size.y};
	im_text("Blocks");
	if (m_blocks.empty()) { im_text("[none]"); }
	for (std::size_t index = 0; index < m_blocks.size(); ++index) {
		auto erased = false;
		auto& block = m_blocks.at(index);
		if (ImGui::TreeNode(FixedString{"{}###{}", block.block.id, index}.c_str())) {
			block_control(block, index);

			if (ImGui::SmallButton("Remove")) {
				m_blocks.erase(m_blocks.begin() + static_cast<std::ptrdiff_t>(index));
				erased = true;
			}

			ImGui::TreePop();
			if (erased) { break; }
		}

		auto local_origin = 0.5f * glm::vec2{block.block.rect.lt + block.block.rect.rb};
		local_origin.y = -local_origin.y;
		auto rect = LineRect{};
		rect.origin = origin_offset + local_origin;
		rect.size = glm::vec2{block.block.rect.rb - block.block.rect.lt};
		block.rect.set_geometry(Geometry::from(rect));
	}

	ImGui::Separator();
	if (ImGui::Button("Generate Blocks")) { ImGui::OpenPopup("Generate"); }

	if (ImGui::BeginPopup("Generate")) {
		ImGui::SetNextItemWidth(30.0f);
		ImGui::DragInt("rows", &m_tile_count.y, 1.0f, 1, 1000);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(30.0f);
		ImGui::DragInt("cols", &m_tile_count.x, 1.0f, 1, 1000);
		if (ImGui::Button("Generate")) {
			generate_blocks();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Tiler::block_control(Block& out, std::size_t const index) const {
	glm::ivec2 const size = m_sprite->get_size();

	if (out.id("id")) {
		out.block.id = out.id.as_view();
		if (out.block.id.empty()) {
			out.block.id = std::to_string(index);
			out.id.set_text(out.block.id);
		}
	}

	ImGui::DragInt("left", &out.block.rect.lt.x, 1.0f, 0, out.block.rect.rb.x);
	ImGui::DragInt("top", &out.block.rect.lt.y, 1.0f, 0, out.block.rect.rb.y);
	ImGui::DragInt("right", &out.block.rect.rb.x, 1.0f, out.block.rect.lt.x, size.x);
	ImGui::DragInt("bottom", &out.block.rect.rb.y, 1.0f, out.block.rect.lt.y, size.y);

	auto const rect_size = out.block.rect.rb - out.block.rect.lt;
	if (drag_ivec2("position", out.block.rect.lt, {.hi = size - rect_size})) { out.block.rect.rb = out.block.rect.lt + rect_size; }

	auto rgba = out.rect.tint.to_vec4();
	if (ImGui::ColorEdit3("RGB", &rgba.x)) { out.rect.tint = Rgba::from(rgba); }
}

void Tiler::metadata_control() {
	auto const size = [&]() -> glm::ivec2 {
		if (auto const& texture = m_sprite->get_texture()) { return texture->get_size(); }
		return {};
	}();
	image_meta_control(m_image_uri, size);
}

auto Tiler::load_uri(std::string_view const uri) -> bool {
	if (!get_app().get_data_store().exists(uri)) { return false; }

	auto const extension = fs::path{uri}.extension().string();
	if (extension == ".json") { return load_atlas(uri); }
	return load_image_at(uri);
}

void Tiler::load_previous() {
	if (load_uri(state->tiler.last_loaded)) { return; }
	state->tiler.last_loaded.clear();
	save_state();
	new_atlas();
}

auto Tiler::load_image_at(std::string_view const uri) -> bool {
	auto texture = m_loader.load_texture(uri);
	if (!texture) { return false; }

	glm::vec2 const image_size = texture->get_size();
	m_sprite->set_texture(std::move(texture));
	m_sprite->set_size(image_size);

	m_blocks.clear();
	m_image_uri = uri;
	m_json_uri.clear();

	main_view.scale = auto_zoom(image_size);
	set_title();

	m_log.info("loaded '{}'", uri);
	return true;
}

void Tiler::new_atlas() {
	m_sprite->set_texture({});
	m_sprite->set_size(Quad::size_v);

	m_image_uri.clear();
	m_json_uri.clear();
	m_unsaved = false;

	main_view.scale = auto_zoom(m_sprite->get_size());
	set_title();
}

auto Tiler::load_atlas(std::string_view const uri) -> bool {
	auto json = m_loader.load_json(uri);
	if (!json || !json.contains("image") || !json.contains("blocks")) { return false; }

	if (!load_image_at(json["image"].as_string())) { return false; }

	m_blocks.clear();
	for (auto const& in_block : json["blocks"].array_view()) {
		auto out_block = TextureAtlas::Block{};
		out_block.id = in_block["id"].as_string();
		from_json(in_block["rect"], out_block.rect);
		m_blocks.push_back(make_block(std::move(out_block)));
	}
	m_unsaved = false;

	m_json_uri = uri;
	set_title();

	m_log.info("loaded '{}'", uri);

	return true;
}

void Tiler::save_atlas() {
	if (m_json_uri.empty()) { return; }
	auto json = dj::Json{};
	json["image"] = m_image_uri;
	auto& out_blocks = json["blocks"];
	for (auto const& in_block : m_blocks) {
		auto out_block = dj::Json{};
		out_block["id"] = in_block.block.id;
		to_json(out_block["rect"], in_block.block.rect);
		out_blocks.push_back(std::move(out_block));
	}

	if (!save_json(json, m_json_uri)) {
		m_log.error("failed to save TextureAtlas to '{}'", m_json_uri);
		return;
	}

	state->tiler.last_loaded = m_json_uri;
	save_state();

	m_log.info("saved TextureAtlas to '{}'", m_json_uri);
	m_unsaved = false;
	set_title();
}

void Tiler::generate_blocks() {
	m_tile_count.x = std::max(m_tile_count.x, 1);
	m_tile_count.y = std::max(m_tile_count.y, 1);
	auto const rects = TextureAtlas::make_rects(m_sprite->get_size(), m_tile_count);
	m_blocks.clear();
	auto id = int{};
	for (auto const& rect : rects) { m_blocks.push_back(make_block(id++, rect)); }
}

auto Tiler::make_block(int id, Rect<int> const& rect) const -> Block {
	auto ret = Block{};
	ret.block.rect = rect;
	ret.block.id = std::to_string(id);
	ret.id.set_text(ret.block.id);
	ret.rect.tint = m_block_rgba;
	return ret;
}

auto Tiler::make_block(TextureAtlas::Block in) const -> Block {
	auto ret = Block{.block = std::move(in)};
	ret.id.set_text(ret.block.id);
	ret.rect.tint = m_block_rgba;
	return ret;
}

void Tiler::set_title() { get_app().set_title(format_title("Tiler", m_json_uri, m_unsaved).c_str()); }
} // namespace bave::tools
