#pragma once
#include <imgui.h>
#include <bave/core/fixed_string.hpp>

namespace bave {
template <std::size_t Capacity = 64, typename... Args>
void im_text(fmt::format_string<Args...> fmt, Args&&... args) {
	ImGui::Text("%s", FixedString<Capacity>{fmt, std::forward<Args>(args)...}.c_str()); // NOLINT(cppcoreguidelines-pro-type-vararg)
}
} // namespace bave
