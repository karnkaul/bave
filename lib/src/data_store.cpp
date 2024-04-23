#include <bave/data_store.hpp>
#include <djson/json.hpp>
#include <algorithm>
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

void DataStore::add_loader(std::unique_ptr<IDataLoader> loader, int const priority) {
	if (!loader) { return; }
	auto const it = std::lower_bound(m_loaders.begin(), m_loaders.end(), priority, [](Entry const& a, int const priority) { return a.priority < priority; });
	auto entry = Entry{.loader = std::move(loader), .priority = priority};
	m_loaders.insert(it, std::move(entry));
}

auto DataStore::exists(std::string_view uri) const -> bool {
	if (uri.empty()) { return false; }
	for (auto const& [loader, _] : m_loaders) {
		if (loader->exists(uri)) { return true; }
	}
	return false;
}

auto DataStore::read_bytes(std::string_view uri) const -> std::vector<std::byte> {
	if (uri.empty()) { return {}; }
	auto ret = std::vector<std::byte>{};
	for (auto const& [loader, _] : m_loaders) {
		if (loader->read_bytes(ret, uri)) { return ret; }
	}
	m_log.warn("failed to load data at: '{}'", uri);
	return {};
}

auto DataStore::read_string(std::string_view uri) const -> std::string {
	if (uri.empty()) { return {}; }
	auto ret = std::string{};
	for (auto const& [loader, _] : m_loaders) {
		if (loader->read_string(ret, uri)) { return ret; }
	}
	m_log.warn("failed to load data at: '{}'", uri);
	return {};
}

auto DataStore::read_json(std::string_view uri) const -> dj::Json {
	auto str = read_string(uri);
	if (str.empty()) { return {}; }
	return dj::Json::parse(str);
}

auto DataStore::to_spir_v(std::string_view const glsl) const -> std::string {
	auto spir_v_uri = make_spir_v_path(glsl);
	if (!exists(spir_v_uri)) {
		m_log.warn("SPIR-V not found: '{}'", spir_v_uri);
		return {};
	}
	return spir_v_uri;
}
} // namespace bave
