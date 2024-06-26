#include <bave/io/json_io.hpp>
#include <tools/tiler.hpp>
#include <algorithm>
#include <filesystem>

namespace bave::tools {
namespace fs = std::filesystem;

Tiler::Tiler(App& app, NotNull<std::shared_ptr<State>> const& state)
	: Applet(app, state), m_loader(&get_app().get_data_store(), &get_app().get_render_device()) {
	load_previous();

	Shader::default_line_width = 3.0f;
}

void Tiler::tick() {
	Applet::tick();

	begin_sidepanel_window("Sprite Sheet");

	if (ImGui::CollapsingHeader("Tiles", ImGuiTreeNodeFlags_DefaultOpen)) { tiles_control(); }

	if (ImGui::CollapsingHeader("Metadata", ImGuiTreeNodeFlags_DefaultOpen)) { metadata_control(); }

	if (ImGui::CollapsingHeader("Misc")) {
		clear_colour_control();
		zoom_control("Zoom", main_view.scale);
	}

	ImGui::End();
}

void Tiler::render(Shader& shader) const {
	m_sprite.draw(shader);

	glm::ivec2 const size = m_sprite.get_size();
	auto const origin_offset = 0.5f * glm::vec2{-size.x, size.y};

	auto outline = LineRectShape{};
	outline.tint = m_tile_rgba;

	for (auto const& block : m_blocks) {
		auto local_origin = 0.5f * glm::vec2{block.tile.image_rect.lt + block.tile.image_rect.rb};
		local_origin.y = -local_origin.y;
		auto rect = LineRect{};
		rect.origin = origin_offset + local_origin;
		rect.size = glm::vec2{block.tile.image_rect.rb - block.tile.image_rect.lt};
		outline.set_shape(rect);
		outline.draw(shader);
	}

	outline.tint = m_collider_rgba;
	for (auto const& block : m_blocks) {
		for (auto const& collider : block.tile.colliders) {
			auto local_origin = 0.5f * glm::vec2{collider.lt + collider.rb};
			local_origin.y = -local_origin.y;
			auto rect = LineRect{};
			rect.origin = origin_offset + local_origin;
			rect.size = glm::vec2{collider.rb - collider.lt};
			outline.set_shape(rect);
			outline.draw(shader);
		}
	}
}

void Tiler::file_menu_items() {
	if (ImGui::MenuItem("New")) { new_atlas(); }
	if (ImGui::MenuItem("Open...")) {
		if (auto uri = dialog_open_file("Open Image / TileSheet"); !uri.empty()) { load_new_uri(uri); }
	}
	if (ImGui::MenuItem("Save", nullptr, false, !m_json_uri.empty())) { save_atlas(); }
	if (ImGui::MenuItem("Save As...", nullptr, false, !m_image_uri.empty())) {
		auto json_uri = fs::path{m_json_uri};
		if (json_uri.empty()) { json_uri = replace_extension(m_image_uri, ".sheet.json"); }
		if (auto uri = dialog_save_file("Save TileSheet", json_uri.generic_string()); !uri.empty()) {
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
	auto rgba = m_tile_rgba.to_vec4();
	if (ImGui::ColorEdit3("RGB", &rgba.x)) { m_tile_rgba = Rgba::from(rgba); }
	if (ImGui::Button("Add Tile")) { m_blocks.push_back(make_block(static_cast<int>(m_blocks.size()))); }

	ImGui::Separator();
	im_text("Tiles");
	if (m_blocks.empty()) { im_text("[none]"); }
	auto modified = false;
	for (std::size_t index = 0; index < m_blocks.size(); ++index) {
		auto erased = false;
		auto& block = m_blocks.at(index);
		if (ImGui::TreeNode(FixedString{"{}###{}", block.tile.id, index}.c_str())) {
			modified |= block_control(block, index);

			if (ImGui::SmallButton("Remove")) {
				m_blocks.erase(m_blocks.begin() + static_cast<std::ptrdiff_t>(index));
				modified = true;
				erased = true;
			}

			ImGui::TreePop();
			if (erased) { break; }
		}
	}

	ImGui::Separator();
	if (ImGui::Button("Generate Tiles")) { ImGui::OpenPopup("Generate"); }

	if (ImGui::BeginPopup("Generate")) {
		ImGui::SetNextItemWidth(30.0f);
		ImGui::DragInt("rows", &m_tile_count.y, 1.0f, 1, 1000);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(30.0f);
		ImGui::DragInt("cols", &m_tile_count.x, 1.0f, 1, 1000);
		if (ImGui::Button("Generate")) {
			generate_blocks();
			modified = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (modified && !m_unsaved) {
		m_unsaved = true;
		set_title();
	}
}

auto Tiler::block_control(Block& out, std::size_t const index) const -> bool {
	glm::ivec2 const size = m_sprite.get_size();

	auto ret = false;
	if (out.id("id")) {
		ret = true;
		out.tile.id = out.id.as_view();
		if (out.tile.id.empty()) {
			out.tile.id = std::to_string(index);
			out.id.set_text(out.tile.id);
		}
	}

	if (ImGui::TreeNode("image rect")) {
		auto const range = InclusiveRange<Rect<int>>{
			.lo = {.lt = {0, 0}, .rb = out.tile.image_rect.lt},
			.hi = {.lt = out.tile.image_rect.rb, .rb = size},
		};
		ret |= drag_irect(out.tile.image_rect, range);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("colliders")) {
		if (ImGui::SmallButton("add")) {
			out.tile.colliders.push_back(out.tile.image_rect);
			ret = true;
		}

		for (std::size_t i = 0; i < out.tile.colliders.size(); ++i) {
			if (ImGui::TreeNode(FixedString{"{}", i}.c_str())) {
				ret |= drag_irect(out.tile.colliders.at(i));
				if (ImGui::SmallButton("remove")) {
					out.tile.colliders.erase(out.tile.colliders.begin() + static_cast<std::ptrdiff_t>(i));
					ret = true;
					ImGui::TreePop();
					break;
				}
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	return ret;
}

void Tiler::metadata_control() {
	auto const size = [&]() -> glm::ivec2 {
		if (auto const& texture = m_sprite.get_texture()) { return texture->get_size(); }
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
	m_sprite.set_texture(std::move(texture));
	m_sprite.set_size(image_size);

	m_blocks.clear();
	m_image_uri = uri;
	m_json_uri.clear();

	main_view.scale = auto_zoom(image_size);
	set_title();

	m_log.info("loaded '{}'", uri);
	return true;
}

void Tiler::new_atlas() {
	m_blocks.clear();

	m_image_uri.clear();
	m_json_uri.clear();
	m_unsaved = false;

	state->tiler.last_loaded.clear();
	save_state();

	main_view.scale = auto_zoom(m_sprite.get_size());
	set_title();
}

auto Tiler::load_atlas(std::string_view const uri) -> bool {
	auto json = m_loader.load_json_asset<TextureAtlas>(uri);
	if (!json) { return false; }

	if (!load_image_at(json["image"].as_string())) { return false; }

	auto sheet = TileSheet{};
	from_json(json["tile_sheet"], sheet);

	m_blocks.clear();
	for (auto& tile : sheet.tiles) { m_blocks.push_back(make_block(std::move(tile))); }
	m_unsaved = false;

	m_json_uri = uri;
	set_title();

	m_log.info("loaded '{}'", uri);

	return true;
}

void Tiler::save_atlas() {
	if (m_json_uri.empty()) { return; }
	auto json = dj::Json{};
	json["asset_type"] = get_asset_type<TextureAtlas>();
	json["image"] = m_image_uri;
	auto in_sheet = TileSheet{};
	for (auto const& block : m_blocks) { in_sheet.tiles.push_back(block.tile); }
	auto& out_sheet = json["tile_sheet"];
	to_json(out_sheet, in_sheet);

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
	auto const rects = TileSheet::make_rects(m_sprite.get_size(), m_tile_count);
	m_blocks.clear();
	auto id = int{};
	for (auto const& rect : rects) { m_blocks.push_back(make_block(id++, rect)); }
}

auto Tiler::make_block(int id, Rect<int> const& rect) -> Block {
	auto ret = Block{};
	ret.tile.image_rect = rect;
	ret.tile.id = std::to_string(id);
	ret.id.set_text(ret.tile.id);
	return ret;
}

auto Tiler::make_block(TileSheet::Tile in) -> Block {
	auto ret = Block{.tile = std::move(in)};
	ret.id.set_text(ret.tile.id);
	return ret;
}

void Tiler::set_title() { get_app().set_title(format_title("Tiler", m_json_uri, m_unsaved).c_str()); }
} // namespace bave::tools
