#pragma once
#include <bave/core/inclusive_range.hpp>
#include <bave/game.hpp>
#include <bave/graphics/drawable.hpp>
#include <tools/state.hpp>
#include <functional>
#include <unordered_map>
#include <pfd/portable-file-dialogs.hpp>

namespace bave::tools {
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

	void tick() override;
	void render() const override;

	void on_key(KeyInput const& key_input) override;
	void on_scroll(MouseScroll const& scroll) override;

	static void begin_lt_window(CString name, bool resizeable);
	void begin_fullscreen_window(CString name) const;
	void begin_sidepanel_window(CString name, float min_width = 300.0f);

	void zoom_control();
	void wireframe_control();

	void save_state();

	[[nodiscard]] auto get_sidepanel_width() const -> float { return m_sidepanel_width; }

	virtual void file_menu_items();
	virtual void main_menu() {}

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
