#pragma once
#include <imgui.h>
#include <bave/core/c_string.hpp>
#include <bave/core/polymorphic.hpp>
#include <span>
#include <string_view>
#include <vector>

namespace bave {
class ImInputText : public Polymorphic {
  public:
	static constexpr std::size_t init_size_v{64};

	auto update(CString name) -> bool;
	void set_text(std::string_view text);

	auto operator()(CString name) -> bool { return update(name); }

	[[nodiscard]] auto as_view() const -> std::string_view { return m_buffer.data(); }
	[[nodiscard]] auto as_span() const -> std::span<char const> { return m_buffer; }

	int flags{};

  protected:
	virtual auto on_callback(ImGuiInputTextCallbackData& data) -> int;

	void resize_buffer(ImGuiInputTextCallbackData& data);

	std::vector<char> m_buffer{};
};
} // namespace bave
