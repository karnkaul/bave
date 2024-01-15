#include <bave/data_store.hpp>
#include <djson/json.hpp>
#include <filesystem>

namespace bave {
namespace {
namespace fs = std::filesystem;

auto make_spir_v_path(std::string_view const uri) -> std::string {
	if (fs::path{uri}.extension() == ".spv") { return std::string{uri}; }
	return fmt::format("{}.spv", uri);
}
} // namespace

auto DataStore::as_string_view(std::span<std::byte const> bytes) -> std::string_view {
	return {reinterpret_cast<char const*>(bytes.data()), bytes.size()}; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}

auto DataStore::read_bytes(std::string_view uri) const -> std::vector<std::byte> {
	if (uri.empty()) { return {}; }
	auto path = make_full_path(uri);
	auto ret = std::vector<std::byte>{};
	if (!do_read_bytes(ret, path.c_str())) {
		m_log.warn("failed to load data at: '{}'", uri);
		return {};
	}
	return ret;
}

auto DataStore::read_string(std::string_view uri) const -> std::string {
	if (uri.empty()) { return {}; }
	auto path = make_full_path(uri);
	auto ret = std::string{};
	if (!do_read_string(ret, path.c_str())) {
		m_log.warn("failed to load data at: '{}'", uri);
		return {};
	}
	return ret;
}

auto DataStore::read_json(std::string_view uri) const -> dj::Json {
	auto str = read_string(uri);
	if (str.empty()) { return {}; }
	return dj::Json::parse(str);
}

auto DataStore::make_full_path(std::string_view uri) const -> std::string { return (fs::path{m_prefix} / uri).generic_string(); }

auto DataStore::make_uri(std::string_view const full_path) const -> std::string {
	if (full_path.empty()) { return {}; }
	return fs::path{full_path}.lexically_relative(get_mount_point()).generic_string();
}

auto DataStore::to_spir_v(std::string_view const glsl) const -> std::string {
	auto spir_v_uri = make_spir_v_path(glsl);
	if (!exists(spir_v_uri)) {
		m_log.warn("SPIR-V not found: '{}'", spir_v_uri);
		return {};
	}
	return spir_v_uri;
}

auto DataStore::do_read_string(std::string& out, CString path) const -> bool {
	auto bytes = std::vector<std::byte>{};
	if (!do_read_bytes(bytes, path)) { return false; }
	out = std::string{reinterpret_cast<char const*>(bytes.data()), bytes.size()}; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
	return true;
}
} // namespace bave
