#pragma once
#include <bave/graphics/sprite.hpp>
#include <bave/imgui/im_input_text.hpp>
#include <bave/loader.hpp>
#include <tools/applet.hpp>

namespace bave::tools {
class Animator : public Applet {
	static constexpr auto quad_size_v = glm::vec2{300.0f};

	void tick() final;
	void render(Shader& shader) const final;
	void change_zoom(float delta, glm::vec2 cursor_position) final;
	auto load_new_uri(std::string_view uri) -> bool final;

	void file_menu_items() final;

	[[nodiscard]] auto can_load_anim() const -> bool { return m_texture != nullptr; }

	void animation_control();
	[[nodiscard]] auto get_next_tile_id() const -> std::string;
	void metadata_control();
	void misc_control();
	auto tiles_control() -> bool;

	auto load_previous() -> bool;

	auto load_atlas(std::string_view uri) -> bool;
	void load_new_atlas(std::string_view uri);

	void new_animation();
	auto load_animation(std::string_view uri) -> bool;
	void load_new_animation(std::string_view uri);
	void save_animation();

	void setup_scene();
	void set_title();

	Logger m_log{"Animator"};
	Loader m_loader;

	CustomShape m_rect;
	QuadShape m_separator;
	QuadShape m_image_quad;
	SpriteAnim m_sprite;
	std::shared_ptr<TextureAtlas> m_texture{};

	Transform m_top_view{};
	Transform m_bottom_view{};

	std::vector<std::string> m_tile_ids{};

	bool m_unsaved{};

  public:
	Animator(App& app, NotNull<std::shared_ptr<State>> const& state);
};
} // namespace bave::tools
