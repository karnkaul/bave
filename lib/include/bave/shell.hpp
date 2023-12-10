#pragma once
#include <bave/core/c_string.hpp>

namespace bave::shell {
[[nodiscard]] auto exec_cmd(CString command) -> bool;
[[nodiscard]] auto exec_cmd_silent(CString command) -> bool;
} // namespace bave::shell
