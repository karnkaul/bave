#include <bave/file_io.hpp>
#include <platform/desktop/desktop_data_store.hpp>
#include <filesystem>

namespace bave {
namespace fs = std::filesystem;

DesktopDataStore::DesktopDataStore(std::string_view data_dir) : DataStore("DesktopDataStore") {
	if (!set_mount_point(data_dir)) { m_log.warn("passed data_dir is not a directory: '{}'", data_dir); }
}

auto DesktopDataStore::do_exists(CString const path) const -> bool { return does_exist(path); }

auto DesktopDataStore::do_read_bytes(std::vector<std::byte>& out, CString const path) const -> bool { return read_bytes_from(out, path); }

auto DesktopDataStore::do_read_string(std::string& out, CString const path) const -> bool { return read_string_from(out, path); }

auto DesktopDataStore::set_mount_point(std::string_view directory) -> bool {
	if (!fs::is_directory(directory)) { return false; }
	m_prefix = directory;
	m_log.info("mounted: '{}'", m_prefix);
	return true;
}
} // namespace bave
