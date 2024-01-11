#pragma once
#include <bave/driver.hpp>
#include <tools/applet.hpp>

namespace bave::tools {
class Runner : public Driver {
	void tick() override;
	void render() const final;

	void on_key(KeyInput const& key_input) override;
	void on_scroll(MouseScroll const& scroll) override;

	[[nodiscard]] auto load_active() const -> std::unique_ptr<Applet>;

	void save_state();

	void main_menu_bar();
	void applet_menu();

	NotNull<std::shared_ptr<State>> m_state;
	std::unordered_map<std::string_view, std::function<std::unique_ptr<Applet>()>> m_map{};
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
