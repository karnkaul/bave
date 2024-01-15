#pragma once
#include <bave/graphics/sprite.hpp>
#include <bave/imgui/im_input_text.hpp>
#include <bave/loader.hpp>
#include <tools/applet.hpp>

namespace bave::tools {
class Tiler : public Applet {
	struct Block {
		CustomShape rect;
		TextureAtlas::Block block{};
		ImInputText id{};
	};

	static constexpr auto rect_v = Rect<int>{.rb = glm::ivec2{200}};

	void tick() final;
	void render(Shader& shader) const final;

	void file_menu_items() final;
	auto load_new_uri(std::string_view uri) -> bool final;

	void tiles_control();
	void block_control(Block& out, std::size_t index) const;
	void metadata_control();

	auto load_uri(std::string_view uri) -> bool;
	void load_previous();

	auto load_image_at(std::string_view uri) -> bool;

	void new_atlas();
	auto load_atlas(std::string_view uri) -> bool;
	void save_atlas();
	void generate_blocks();

	[[nodiscard]] auto make_block(int id, Rect<int> const& rect = rect_v) const -> Block;
	[[nodiscard]] auto make_block(TextureAtlas::Block in) const -> Block;

	void set_title();

	Logger m_log{"Tiler"};
	Loader m_loader;

	Ptr<Sprite> m_sprite{};
	std::vector<Block> m_blocks{};

	std::string m_image_uri{};
	std::string m_json_uri{};

	Rgba m_block_rgba = red_v;
	glm::ivec2 m_tile_count{1, 1};
	float m_outline_width{3.0f};
	bool m_unsaved{};

  public:
	Tiler(App& app, NotNull<std::shared_ptr<State>> const& state);
};
} // namespace bave::tools
