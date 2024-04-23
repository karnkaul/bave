#pragma once
#include <bave/data_loader.hpp>
#include <string>

namespace bave::zip {
/// \brief Mount a ZIP archive into ZIP VFS.
/// \param name Name to mount as: must be unique.
/// \param zip_bytes Bytes of ZIP archive.
/// \returns true on success.
[[nodiscard]] auto mount(std::string name, std::vector<std::byte> zip_bytes) -> bool;

/// \brief Unmount a ZIP archive from ZIP VFS.
/// \param name Name of mounted archive.
/// \returns true on success.
[[nodiscard]] auto unmount(std::string const& name) -> bool;

/// \brief Check if a ZIP archive is mounted in ZIP VFS.
/// \param name Name of the archive.
/// \returns true if mounted.
[[nodiscard]] auto is_mounted(std::string_view name) -> bool;

/// \brief Check if a file exists within ZIP VFS.
/// \param path Path to check for.
/// \returns true if file exists in a mounted archive.
[[nodiscard]] auto exists(CString path) -> bool;

/// \brief Read bytes from a file within ZIP VFS.
/// \param out Storage for bytes to be read.
/// \param path Path to read from.
/// \returns true on success.
[[nodiscard]] auto read_bytes(std::vector<std::byte>& out, CString path) -> bool;
} // namespace bave::zip
