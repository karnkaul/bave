#include <tools/animator.hpp>
#include <tools/nine_slicer.hpp>
#include <tools/runner.hpp>
#include <tools/tiler.hpp>

namespace bave::tools {
namespace {
[[nodiscard]] auto load_or_create_state() {
	static auto const logger = bave::Logger{"BaveTools"};
	auto ret = std::make_shared<State>();
	if (ret->load()) {
		logger.info("loaded State from '{}'", State::path_v.as_view());
	} else {
		ret->save();
		logger.info("created State at '{}'", State::path_v.as_view());
	}
	return ret;
}
} // namespace

Runner::Runner(App& app) : Driver(app), m_state(load_or_create_state()) {
	app.set_title("Bave Tools");

	m_map = {
		{"Tiler", [&] { return std::make_unique<Tiler>(get_app(), m_state); }},
		{"NineSlicer", [&] { return std::make_unique<NineSlicer>(get_app(), m_state); }},
		{"Animator", [&] { return std::make_unique<Animator>(get_app(), m_state); }},
	};

	m_active = load_active();
}

void Runner::tick() {
	main_menu_bar();
	m_active->tick();
}

void Runner::render() const { m_active->render(); }

void Runner::on_key(KeyInput const& key_input) {
	if (key_input.action == Action::ePress && key_input.key == Key::eW && key_input.mods == make_key_mods(mod::ctrl)) { get_app().shutdown(); }
}

void Runner::on_scroll(MouseScroll const& scroll) { m_active->on_scroll(scroll); }

auto Runner::load_active() const -> std::unique_ptr<Applet> {
	if (auto const it = m_map.find(m_state->active_applet); it != m_map.end()) { return it->second(); }
	return std::make_unique<Tiler>(get_app(), m_state);
}

void Runner::main_menu_bar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			m_active->file_menu_items();
			ImGui::EndMenu();
		}

		m_active->main_menu();
		applet_menu();

		ImGui::EndMainMenuBar();
	}
}

void Runner::applet_menu() {
	if (ImGui::BeginMenu("Applets")) {
		for (auto const& [name, factory] : m_map) {
			if (ImGui::MenuItem(name.data())) {
				m_log.info("loading '{}'", name);
				m_state->active_applet = name;
				m_active->save_state();
				m_active = factory();
			}
		}
		ImGui::EndMenu();
	}
}
} // namespace bave::tools
