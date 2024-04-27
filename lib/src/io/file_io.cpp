#include <bave/io/file_io.hpp>
#include <array>
#include <filesystem>
#include <fstream>

namespace bave {
namespace {
namespace fs = std::filesystem;

template <typename Type>
auto read_data(Type& out, bave::CString path) -> bool {
	if (path.as_view().empty()) { return false; }
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

auto file::upfind(std::string_view const base, std::string_view const patterns) -> std::string {
	auto const base_paths = std::array{fs::absolute(base), fs::current_path()};

	for (auto const& base_path : base_paths) {
		if (base_path.empty()) { continue; }
		auto parser = PatternParser{patterns};
		auto pattern = std::string_view{};
		while (parser.next(pattern)) {
			for (auto path = base_path; !path.empty() && path.parent_path() != path; path = path.parent_path()) {
				if (auto ret = path / pattern; fs::exists(ret)) { return ret.make_preferred().generic_string(); }
			}
		}
	}

	return {};
}

auto file::upfind_dir(std::string_view const base, std::string_view const patterns) -> std::string {
	auto ret = upfind(base, patterns);
	if (fs::is_directory(ret)) { return ret; }
	return {};
}

auto file::upfind_parent(std::string_view const base, std::string_view const filename) -> std::string {
	return fs::path{upfind(base, filename)}.parent_path().generic_string();
}

auto file::exists(CString const path) -> bool { return fs::exists(path.as_view()); }
auto file::read_bytes(std::vector<std::byte>& out, CString const path) -> bool { return read_data(out, path); }
auto file::read_string(std::string& out, CString const path) -> bool { return read_data(out, path); }

auto file::write_bytes(CString const path, std::span<std::byte const> data) -> bool {
	auto f = std::ofstream{path.c_str(), std::ios::binary};
	if (!f) { return false; }
	if (data.empty()) { return true; }
	auto const size = static_cast<std::streamsize>(data.size());
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	return static_cast<bool>(f.write(reinterpret_cast<char const*>(data.data()), size));
}

auto file::write_string(CString const path, std::string_view text) -> bool {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	return write_bytes(path, std::span{reinterpret_cast<std::byte const*>(text.data()), text.size()});
}

auto file::make_uri(std::string_view const assets_dir, std::string_view const full_path) -> std::string {
	if (full_path.empty()) { return {}; }
	if (assets_dir.empty()) { return std::string{full_path}; }
	return fs::path{full_path}.lexically_relative(assets_dir).generic_string();
}
} // namespace bave
