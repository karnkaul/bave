#include <bave/logger.hpp>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>

namespace bave {
namespace {
using namespace std::chrono_literals;

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
} // namespace

auto log::format_full(char const level, std::string_view const tag, std::string_view const message) -> std::string {
	auto const tid = get_thread_id();
	auto const timestamp = get_timestamp();
	return fmt::format("[{}][{: >2}{}] [{}] {} [{}]\n", level, 'T', tid, tag, message, timestamp.data());
}

auto log::format_thread(std::string_view message) -> std::string {
	auto const tid = get_thread_id();
	return fmt::format("[{: >2}{}] {}\n", 'T', tid, message);
}

void log::log_message(char const level, CString const tag, CString const message) { internal::log_message(level, tag, message); }

auto log::get_thread_id() -> int {
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

void Logger::log(char const level, CString const message) const { log::log_message(level, tag.c_str(), message); }
} // namespace bave
