#pragma once
#include <bave/core/c_string.hpp>
#include <string>

namespace dj {
class Json;
}

namespace bave::tools {
struct State {
	static constexpr auto path_v = CString{"bave-tools.state.json"};

	std::string active_applet{};

	struct {
		std::string last_loaded{};
	} nine_slicer{};

	struct {
		std::string last_loaded{};
	} tiler{};

	auto load(CString path = path_v) -> bool;
	auto save(CString path = path_v) const -> bool; // NOLINT(modernize-use-nodiscard)

	void load_members(dj::Json const& json);
	void save_members(dj::Json& out) const;
};
} // namespace bave::tools
