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

	if (auto const& in_ns = json["nine_slicer"]) { nine_slicer.last_loaded = in_ns["last_loaded"].as_string(); }

	if (auto const& in_tl = json["tiler"]) { tiler.last_loaded = in_tl["last_loaded"].as_string(); }
}

void State::save_members(dj::Json& out) const {
	out["active_applet"] = active_applet;

	auto& out_ns = out["nine_slicer"];
	if (!nine_slicer.last_loaded.empty()) { out_ns["last_loaded"] = nine_slicer.last_loaded; }

	auto& out_tl = out["tiler"];
	if (!tiler.last_loaded.empty()) { out_tl["last_loaded"] = tiler.last_loaded; }
}
} // namespace bave::tools
