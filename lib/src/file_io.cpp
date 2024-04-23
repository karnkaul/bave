#include <bave/file_io.hpp>
#include <array>
#include <filesystem>
#include <fstream>

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

auto bave::find_super_dir(std::string_view base, std::string_view patterns) -> std::string {
	auto const base_paths = std::array{fs::absolute(base), fs::current_path()};

	for (auto const& base_path : base_paths) {
		auto parser = PatternParser{patterns};
		auto pattern = std::string_view{};
		while (parser.next(pattern)) {
			for (auto path = base_path; !path.empty() && path.parent_path() != path; path = path.parent_path()) {
				if (auto ret = path / pattern; fs::is_directory(ret)) { return ret.make_preferred().generic_string(); }
			}
		}
	}

	return {};
}

auto bave::does_exist(CString const path) -> bool { return fs::exists(path.as_view()); }
auto bave::read_bytes_from(std::vector<std::byte>& out, CString const path) -> bool { return read_data(out, path); }
auto bave::read_string_from(std::string& out, CString const path) -> bool { return read_data(out, path); }
