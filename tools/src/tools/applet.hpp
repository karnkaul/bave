#pragma once
#include <bave/core/inclusive_range.hpp>
#include <bave/driver.hpp>
#include <bave/graphics/drawable.hpp>
#include <bave/imgui/im_text.hpp>
#include <djson/json.hpp>
#include <tools/state.hpp>
#include <pfd/portable-file-dialogs.hpp>

namespace bave::tools {
[[nodiscard]] constexpr auto or_none(std::string_view const in) -> std::string_view {
	if (in.empty()) { return "[none]"; }
	return in;
}

class Applet : public Polymorphic {
  public:
	static constexpr auto window_size_v = glm::ivec2{1280, 720};

	explicit Applet(App& app, NotNull<std::shared_ptr<State>> const& state);

	virtual void tick() {}
	virtual void render() const;

	virtual void on_key(KeyInput const& key_input);
	virtual void on_scroll(MouseScroll const& scroll);
	virtual void on_drop(std::span<std::string const> paths);

	virtual void file_menu_items();
	virtual void main_menu() {}

	virtual auto load_new_uri(std::string_view /*uri*/) -> bool { return false; }

	void save_state();

	Rgba clear_colour{black_v};

  protected:
	static constexpr float y_top_v{20.0f};
	static constexpr auto zoom_scale_range_v = InclusiveRange<glm::vec2>{.lo = glm::vec2{0.01f}, .hi = glm::vec2{5.0f}};

	[[nodiscard]] auto get_app() const -> App& { return *m_app; }
	virtual void render(Shader& default_shader) const;
	virtual void change_zoom(float delta, glm::vec2 cursor_position);

	[[nodiscard]] auto auto_zoom(glm::vec2 content_area, glm::vec2 pad = {500.0f, 200.0f}) const -> glm::vec2;

	static void begin_lt_window(CString label, bool resizeable);
	void begin_fullscreen_window(CString label) const;
	void begin_sidepanel_window(CString label, float min_width = 300.0f);

	static auto drag_ivec2(CString label, glm::ivec2& out, InclusiveRange<glm::ivec2> range = {}, float width = 50.0f) -> bool;
	static auto drag_irect(Rect<int>& out, InclusiveRange<Rect<int>> range = {}, bool positional = true) -> bool;

	void wireframe_control();
	void clear_colour_control();
	static void zoom_control(CString label, glm::vec2& out_scale);
	static void image_meta_control(std::string_view image_uri, glm::ivec2 size);

	[[nodiscard]] auto get_sidepanel_width() const -> float { return m_sidepanel_width; }

	[[nodiscard]] static auto replace_extension(std::string_view uri, std::string_view extension) -> std::string;
	[[nodiscard]] auto truncate_to_uri(std::string_view path) const -> std::string;
	[[nodiscard]] auto dialog_open_file(CString title) const -> std::string;
	[[nodiscard]] auto dialog_save_file(CString title, std::string_view uri) const -> std::string;
	[[nodiscard]] auto save_json(dj::Json const& json, std::string_view uri) const -> bool;
	[[nodiscard]] static auto format_title(std::string_view name, std::string_view uri, bool unsaved) -> std::string;

	template <std::derived_from<Drawable> Type>
	auto push(std::unique_ptr<Type> t) -> Ptr<Type> {
		if (!t) { return {}; }
		auto ret = t.get();
		drawables.push_back(std::move(t));
		return ret;
	}

	NotNull<App*> m_app;
	NotNull<std::shared_ptr<State>> state;

	std::vector<std::unique_ptr<Drawable>> drawables{};
	float zoom_scroll_rate{0.1f};
	Transform main_view{};
	bool wireframe{};

  private:
	Logger m_log{"Applet"};
	float m_sidepanel_width{};
	bool m_ctrl_pressed{};
};
} // namespace bave::tools
