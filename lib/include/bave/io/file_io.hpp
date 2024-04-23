#pragma once
#include <bave/core/c_string.hpp>
#include <string>
#include <vector>

namespace bave::file {
/// \brief Find a super directory that contains a given file/directory pattern.
/// \param base Directory to start search from.
/// \param patterns Comma-separated list of file patterns to search for.
/// \returns Path to super directory if found, else empty string.
[[nodiscard]] auto find_super_dir(std::string_view base, std::string_view patterns) -> std::string;

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
} // namespace bave::file
