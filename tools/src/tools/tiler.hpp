#pragma once
#include <bave/graphics/sprite.hpp>
#include <bave/graphics/tile_sheet.hpp>
#include <bave/imgui/im_input_text.hpp>
#include <bave/loader.hpp>
#include <tools/applet.hpp>

namespace bave::tools {
class Tiler : public Applet {
	struct Block {
		TileSheet::Tile tile{};
		ImInputText id{};
	};

	static constexpr auto rect_v = Rect<int>{.rb = glm::ivec2{200}};

	void tick() final;
	void render(Shader& shader) const final;

	void file_menu_items() final;
	auto load_new_uri(std::string_view uri) -> bool final;

	void tiles_control();
	auto block_control(Block& out, std::size_t index) const -> bool;
	void metadata_control();

	auto load_uri(std::string_view uri) -> bool;
	void load_previous();

	auto load_image_at(std::string_view uri) -> bool;

	void new_atlas();
	auto load_atlas(std::string_view uri) -> bool;
	void save_atlas();
	void generate_blocks();

	[[nodiscard]] static auto make_block(int id, Rect<int> const& rect = rect_v) -> Block;
	[[nodiscard]] static auto make_block(TileSheet::Tile in) -> Block;

	void set_title();

	Logger m_log{"Tiler"};
	Loader m_loader;

	Sprite m_sprite{};
	std::vector<Block> m_blocks{};

	std::string m_image_uri{};
	std::string m_json_uri{};

	Rgba m_tile_rgba = red_v;
	Rgba m_collider_rgba = green_v;
	glm::ivec2 m_tile_count{1, 1};
	float m_outline_width{3.0f};
	bool m_unsaved{};

  public:
	Tiler(App& app, NotNull<std::shared_ptr<State>> const& state);
};
} // namespace bave::tools
