#pragma once
#include <bave/data_loader.hpp>
#include <bave/file_io.hpp>
#include <bave/logger.hpp>
#include <filesystem>

namespace bave {
struct DesktopDataLoader : DataLoader {
	DesktopDataLoader(std::string_view data_dir) {
		if (!set_mount_point(data_dir)) { m_log.warn("passed data_dir is not a directory: '{}'", data_dir); }
	}

	[[nodiscard]] auto exists(CString const path) const -> bool final { return does_exist(path); }

	auto read_bytes(std::vector<std::byte>& out, CString const path) const -> bool final { return read_bytes_from(out, path); }

	auto read_string(std::string& out, CString const path) const -> bool final { return read_string_from(out, path); }

	auto set_mount_point(std::string_view directory) -> bool final {
		if (!std::filesystem::is_directory(directory)) { return false; }
		m_prefix = directory;
		m_log.info("mounted: '{}'", directory);
		return true;
	}

	[[nodiscard]] auto make_full_path(std::string_view uri) const -> std::string { return (m_prefix / uri).generic_string(); }

	Logger m_log{"DesktopDataLoader"};
	std::filesystem::path m_prefix{};
};
} // namespace bave
