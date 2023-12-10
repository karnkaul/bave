#pragma once
#include <string_view>

namespace bave::glslc {
[[nodiscard]] auto is_online() -> bool;
auto compile(std::string_view glsl, std::string_view spir_v) -> bool;
[[nodiscard]] auto make_spir_v_path(std::string_view glsl) -> std::string;
} // namespace bave::glslc
