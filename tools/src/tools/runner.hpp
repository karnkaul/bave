#pragma once
#include <bave/driver.hpp>
#include <tools/applet.hpp>

namespace bave::tools {
class Runner : public Driver {
	void tick() final;
	void render() const final;

	void on_key(KeyInput const& key_input) final;
	void on_scroll(MouseScroll const& scroll) final;
	void on_drop(std::span<std::string const> paths) final;

	[[nodiscard]] auto load_active() -> std::unique_ptr<Applet>;

	void main_menu_bar();
	void applet_menu();

	NotNull<std::shared_ptr<State>> m_state;
	std::unordered_map<std::string_view, std::function<std::unique_ptr<Applet>()>> m_map{};
	std::vector<std::string_view> m_applet_names{};
	std::unique_ptr<Applet> m_active{};
	float m_sidepanel_width{};
	Logger m_log{"Runner"};

  public:
	explicit Runner(App& app);
};

struct Bootloader {
	[[nodiscard]] auto operator()(App& app) const { return std::make_unique<Runner>(app); }
};
} // namespace bave::tools
