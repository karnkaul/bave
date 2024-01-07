#include <bave/file_io.hpp>
#include <platform/desktop/desktop_data_store.hpp>
#include <filesystem>

namespace bave {
namespace fs = std::filesystem;

DesktopDataStore::DesktopDataStore(std::string data_dir) : DataStore("DesktopDataStore") {
	if (!fs::is_directory(data_dir)) {
		m_log.warn("passed data_dir is not a directory: '{}'", data_dir);
		return;
	}

	m_prefix = std::move(data_dir);
	m_log.info("mounted: '{}'", m_prefix);
}

auto DesktopDataStore::do_exists(CString const path) const -> bool { return does_exist(path); }

auto DesktopDataStore::do_read_bytes(std::vector<std::byte>& out, CString const path) const -> bool { return read_bytes_from(out, path); }

auto DesktopDataStore::do_read_string(std::string& out, CString const path) const -> bool { return read_string_from(out, path); }
} // namespace bave
