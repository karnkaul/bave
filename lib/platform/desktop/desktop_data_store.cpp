#include <platform/desktop/desktop_data_store.hpp>
#include <filesystem>
#include <fstream>

namespace bave {
namespace {
namespace fs = std::filesystem;

template <typename Type>
auto do_read_data(Type& out, CString path) -> bool {
	auto file = std::ifstream{path.c_str(), std::ios::ate | std::ios::binary};
	if (!file) { return false; }
	auto const size = file.tellg();
	file.seekg({}, std::ios::beg);
	out.resize(static_cast<size_t>(size));
	file.read(reinterpret_cast<char*>(out.data()), size); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
	return true;
}

struct PatternParser {
	std::string_view remain{};

	constexpr auto next(std::string_view& out) -> bool {
		if (remain.empty()) { return false; }

		auto const i = remain.find_first_of(',');
		if (i == std::string_view::npos) {
			out = remain;
			remain = {};
			return true;
		}

		out = remain.substr(0, i);
		remain = remain.substr(i + 1);
		return true;
	}
};
} // namespace

auto DesktopDataStore::find_super_dir(std::string_view base, std::string_view patterns) -> std::string {
	auto abs_path = fs::absolute(base);
	if (abs_path.empty() || !fs::exists(abs_path)) { abs_path = fs::current_path(); }

	auto parser = PatternParser{patterns};
	auto pattern = std::string_view{};
	while (parser.next(pattern)) {
		for (auto path = abs_path; !path.empty() && path.parent_path() != path; path = path.parent_path()) {
			if (auto ret = path / pattern; fs::is_directory(ret)) { return ret.generic_string(); }
		}
	}

	return {};
}

DesktopDataStore::DesktopDataStore(std::string data_dir) : DataStore("DesktopDataStore") {
	if (!fs::is_directory(data_dir)) {
		m_log.warn("passed data_dir is not a directory: '{}'", data_dir);
		return;
	}

	m_prefix = std::move(data_dir);
	m_log.info("mounted: '{}'", m_prefix);
}

auto DesktopDataStore::do_exists(CString path) const -> bool { return fs::exists(path.as_view()); }

auto DesktopDataStore::do_read_bytes(std::vector<std::byte>& out, CString path) const -> bool { return do_read_data(out, path); }

auto DesktopDataStore::do_read_string(std::string& out, CString path) const -> bool { return do_read_data(out, path); }
} // namespace bave
