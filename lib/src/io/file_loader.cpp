#include <bave/io/file_io.hpp>
#include <bave/io/file_loader.hpp>
#include <filesystem>

namespace bave {
namespace fs = std::filesystem;

FileLoader::FileLoader(std::string_view mount_point) {
	if (!fs::is_directory(mount_point)) {
		m_log.error("mount point not a directory: '{}'", mount_point);
		m_prefix = fs::current_path().generic_string();
		m_log.info("mounting working directory instead: '{}'", m_prefix);
		return;
	}
	m_prefix = mount_point;
}

auto FileLoader::exists(std::string_view const uri) const -> bool { return file::exists(make_full_path(uri).c_str()); }

auto FileLoader::read_bytes(std::vector<std::byte>& out, std::string_view const uri) const -> bool {
	return file::read_bytes(out, make_full_path(uri).c_str());
}

auto FileLoader::read_string(std::string& out, std::string_view const uri) const -> bool { return file::read_string(out, make_full_path(uri).c_str()); }

auto FileLoader::make_full_path(std::string_view uri) const -> std::string { return (fs::path{m_prefix} / uri).generic_string(); }
} // namespace bave
