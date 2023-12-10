#include <fmt/format.h>
#include <bave/shell.hpp>
#include <mutex>

namespace bave {
namespace {
constexpr std::string_view null_v =
#if defined(_WIN32)
	"NUL";
#else
	"/dev/null";
#endif
} // namespace

auto shell::exec_cmd(CString const command) -> bool {
	if (command.as_view().empty()) { return false; }
	static auto s_cout_mutex = std::mutex{};
	auto lock = std::scoped_lock{s_cout_mutex};
	return std::system(command.c_str()) == 0; // NOLINT(concurrency-mt-unsafe)
}

auto shell::exec_cmd_silent(CString const command) -> bool {
	if (command.as_view().empty()) { return false; }
	return exec_cmd(fmt::format("{} >{} 2>&1", command.as_view(), null_v).c_str());
}
} // namespace bave
