#pragma once
#include <bave/core/inclusive_range.hpp>
#include <bave/game.hpp>
#include <bave/graphics/drawable.hpp>
#include <bave/imgui/im_text.hpp>
#include <djson/json.hpp>
#include <tools/state.hpp>
#include <functional>
#include <unordered_map>
#include <pfd/portable-file-dialogs.hpp>

namespace bave::tools {
[[nodiscard]] constexpr auto or_none(std::string_view const in) -> std::string_view {
	if (in.empty()) { return "[none]"; }
	return in;
}

class Applet : public Game {
  public:
	static constexpr std::string_view title_prefix_v{"Bave Tools"};

	template <std::derived_from<Applet> Type>
	static void add_applet(std::string_view name) {
		s_applets.insert_or_assign(name, [](App& app, State state) { return std::make_unique<Type>(app, std::move(state)); });
	}

	static auto make_bootloader(State const& state) -> std::function<std::unique_ptr<Applet>(App&)>;

	explicit Applet(App& app, State state);

  protected:
	static constexpr float y_top_v{20.0f};
	static constexpr auto zoom_range_v = InclusiveRange<int>{1, 500};

	static auto drag_ivec2(CString label, glm::ivec2& out, InclusiveRange<glm::ivec2> range = {}, float width = 50.0f) -> bool;

	void tick() override;
	void render() const final;

	void on_key(KeyInput const& key_input) override;
	void on_scroll(MouseScroll const& scroll) override;

	virtual void render(Shader& shader) const;

	static void begin_lt_window(CString name, bool resizeable);
	void begin_fullscreen_window(CString name) const;
	void begin_sidepanel_window(CString name, float min_width = 300.0f);

	void zoom_control();
	void wireframe_control();
	static void image_meta_control(std::string_view image_uri, glm::ivec2 size);

	void save_state();

	[[nodiscard]] auto get_sidepanel_width() const -> float { return m_sidepanel_width; }

	virtual void file_menu_items();
	virtual void main_menu() {}

	[[nodiscard]] static auto replace_extension(std::string_view uri, std::string_view extension) -> std::string;
	[[nodiscard]] auto truncate_to_uri(std::string_view path) const -> std::string;
	void resize_framebuffer(glm::ivec2 size, glm::ivec2 pad);
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

	State state;

	std::vector<std::unique_ptr<Drawable>> drawables{};
	float zoom{100.0f};
	float zoom_scroll_rate{10.0f};
	glm::vec2 view_position{};
	bool wireframe{};

  private:
	void main_menu_bar();
	void applet_menu();

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	inline static std::unordered_map<std::string_view, std::function<std::unique_ptr<Applet>(App&, State)>> s_applets{};
	float m_sidepanel_width{};
	Logger m_log{"Applet"};
};
} // namespace bave::tools
