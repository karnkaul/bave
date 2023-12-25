#include <bave/logger.hpp>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>

namespace bave {
namespace {
constexpr std::size_t timestamp_size_v{16};

auto get_timestamp() -> std::array<char, timestamp_size_v> {
	auto ret = std::array<char, timestamp_size_v>{};
	auto const time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	static auto mutex = std::mutex{};
	auto lock = std::scoped_lock{mutex};
	auto const* tm = std::localtime(&time); // NOLINT(concurrency-mt-unsafe)
	std::strftime(ret.data(), ret.size() - 1, "%T", tm);
	return ret;
}

auto get_thread_id() -> int {
	struct Map {
		std::thread::id sys_id{};
		int log_id{};
	};

	static auto map = std::vector<Map>{};
	static auto mutex = std::mutex{};
	static auto next_id = int{};

	auto lock = std::scoped_lock{mutex};
	auto const func = [](Map const& m) { return m.sys_id == std::this_thread::get_id(); };
	if (auto it = std::find_if(map.begin(), map.end(), func); it != map.end()) { return it->log_id; }
	auto const id = next_id++;
	map.push_back(Map{.sys_id = std::this_thread::get_id(), .log_id = id});
	return id;
}
} // namespace

auto log::format_message(char const level, std::string_view const tag, std::string_view const message, bool thread_only) -> std::string {
	auto const tid = get_thread_id();
	if (thread_only) { return fmt::format("[{: >2}{}] {} {}\n", 'T', tid, tag, message); }

	auto const timestamp = get_timestamp();
	return fmt::format("[{}][{: >2}{}] [{}] {} [{}]\n", level, 'T', tid, tag, message, timestamp.data());
}

void log::log_message(char const level, CString const tag, CString const message) { internal::log_message(level, tag, message); }

void Logger::log(char const level, CString const message) const { log::log_message(level, tag.c_str(), message); }
} // namespace bave
