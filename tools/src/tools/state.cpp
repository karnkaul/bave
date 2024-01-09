#include <djson/json.hpp>
#include <tools/state.hpp>
#include <filesystem>

namespace bave::tools {
auto State::load(CString path) -> bool {
	if (!std::filesystem::is_regular_file(path.as_view())) { return false; }

	auto const json = dj::Json::from_file(path.c_str());
	if (!json) { return false; }

	load_members(json);
	return true;
}

auto State::save(CString path) const -> bool {
	auto json = dj::Json{};
	save_members(json);
	return json.to_file(path.c_str());
}

void State::load_members(dj::Json const& json) {
	active_applet = json["active_applet"].as_string();
	//
}

void State::save_members(dj::Json& out) const {
	out["active_applet"] = active_applet;
	//
}
} // namespace bave::tools
