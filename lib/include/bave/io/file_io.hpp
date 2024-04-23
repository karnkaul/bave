#pragma once
#include <bave/core/c_string.hpp>
#include <string>
#include <vector>

namespace bave {
[[nodiscard]] auto find_super_dir(std::string_view base, std::string_view patterns) -> std::string;

[[nodiscard]] auto does_exist(CString path) -> bool;
[[nodiscard]] auto read_bytes_from(std::vector<std::byte>& out, CString path) -> bool;
[[nodiscard]] auto read_string_from(std::string& out, CString path) -> bool;
} // namespace bave
