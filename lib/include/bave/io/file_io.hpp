#pragma once
#include <bave/core/c_string.hpp>
#include <span>
#include <string>
#include <vector>

namespace bave::file {
/// \brief Find a super path to a given file/directory pattern.
/// \param base Directory to start search from. The working directory is always searched.
/// \param patterns Comma-separated list of file patterns to search for.
/// \returns Path to file/directory if found, else empty string.
[[nodiscard]] auto upfind(std::string_view base, std::string_view patterns) -> std::string;
/// \brief Find a super path to a given directory pattern.
/// \param base Directory to start search from. The working directory is always searched.
/// \param patterns Comma-separated list of directory name patterns to search for.
/// \returns Path to directory if found, else empty string.
[[nodiscard]] auto upfind_dir(std::string_view base, std::string_view patterns) -> std::string;
/// \brief Find the parent directory of a super directory containing a given file.
/// \param base Directory to start search from. The working directory is always searched.
/// \param filename Filename to search for.
/// \returns Path to parent directory if found, else empty string.
[[nodiscard]] auto upfind_parent(std::string_view base, std::string_view filename) -> std::string;

/// \brief Check if a file exists.
/// \param path Path to check for.
/// \returns true if file exists.
[[nodiscard]] auto exists(CString path) -> bool;

/// \brief Read bytes from a file.
/// \param out Storage for bytes to be read.
/// \param path Path to read from.
/// \returns true on success.
[[nodiscard]] auto read_bytes(std::vector<std::byte>& out, CString path) -> bool;
/// \brief Read string from a file.
/// \param out Storage for string to be read.
/// \param path Path to read from.
/// \returns true on success.
[[nodiscard]] auto read_string(std::string& out, CString path) -> bool;

/// \brief Write bytes to a file.
/// \param path Path to write to.
/// \param data Bytes to write.
/// \returns true on success.
[[nodiscard]] auto write_bytes(CString path, std::span<std::byte const> data) -> bool;
/// \brief Write string to a file.
/// \param path Path to write to.
/// \param text String to write.
/// \returns true on success.
[[nodiscard]] auto write_string(CString path, std::string_view text) -> bool;

/// \brief Create a URI relative to the assets path. Only relevant for desktop platforms.
/// \param assets_dir Path to assets directory.
/// \param full_path Full path to convert to URI.
/// \returns URI relative to the assets path.
[[nodiscard]] auto make_uri(std::string_view assets_dir, std::string_view full_path) -> std::string;
} // namespace bave::file
