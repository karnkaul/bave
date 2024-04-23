#pragma once
#include <bave/data_loader.hpp>
#include <bave/file_io.hpp>
#include <bave/logger.hpp>
#include <filesystem>

namespace bave {
struct DesktopDataLoader : IDataLoader {
	[[nodiscard]] auto exists(std::string_view const uri) const -> bool final { return does_exist(make_full_path(uri).c_str()); }

	auto read_bytes(std::vector<std::byte>& out, std::string_view const uri) const -> bool final { return read_bytes_from(out, make_full_path(uri).c_str()); }

	auto read_string(std::string& out, std::string_view const uri) const -> bool final { return read_string_from(out, make_full_path(uri).c_str()); }

	auto set_mount_point(std::string_view directory) -> bool {
		if (!std::filesystem::is_directory(directory)) { return false; }
		m_prefix = directory;
		return true;
	}

	[[nodiscard]] auto make_full_path(std::string_view uri) const -> std::string { return (m_prefix / uri).generic_string(); }

	std::filesystem::path m_prefix{};
};
} // namespace bave
