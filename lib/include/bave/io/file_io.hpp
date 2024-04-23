#pragma once
#include <bave/core/c_string.hpp>
#include <string>
#include <vector>

namespace bave::file {
[[nodiscard]] auto find_super_dir(std::string_view base, std::string_view patterns) -> std::string;

[[nodiscard]] auto exists(CString path) -> bool;
[[nodiscard]] auto read_bytes(std::vector<std::byte>& out, CString path) -> bool;
[[nodiscard]] auto read_string(std::string& out, CString path) -> bool;
} // namespace bave::file
