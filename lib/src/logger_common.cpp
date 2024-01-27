#include <bave/logger.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace bave {
namespace {
using namespace std::chrono_literals;

constexpr std::size_t timestamp_size_v{16};

constexpr auto to_char(log::Level const level) {
	constexpr auto levels_v = std::array{'E', 'W', 'I', 'D'};
	return levels_v.at(static_cast<std::size_t>(level));
}

auto get_timestamp() -> std::array<char, timestamp_size_v> {
	auto ret = std::array<char, timestamp_size_v>{};
	auto const time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	static auto mutex = std::mutex{};
	auto lock = std::scoped_lock{mutex};
	auto const* tm = std::localtime(&time); // NOLINT(concurrency-mt-unsafe)
	std::strftime(ret.data(), ret.size() - 1, "%T", tm);
	return ret;
}

struct {
	[[nodiscard]] auto get_max_level(std::string_view const tag) const -> log::Level {
		auto lock = std::scoped_lock{mutex};
		if (auto const it = levels.find(tag); it != levels.end()) { return it->second; }
		return fallback;
	}

	void set_max_level(std::string_view const tag, log::Level const level) {
		auto lock = std::scoped_lock{mutex};
		levels.insert_or_assign(tag, level);
	}

	void set_fallback(log::Level const level) {
		auto lock = std::scoped_lock{mutex};
		fallback = level;
	}

  private:
	std::unordered_map<std::string_view, log::Level> levels{};
	log::Level fallback{log::Level::eDebug};
	mutable std::mutex mutex{};
} g_data{}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
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

void log::log_message(Level const level, CString const tag, CString const message) {
	if (g_data.get_max_level(tag) < level) { return; }
	internal::log_message(to_char(level), tag, message);
}

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

void log::set_max_level(Level const level) { g_data.set_fallback(level); }

void log::set_max_level(std::string_view const tag, Level const level) {
	if (tag.empty()) { return; }
	g_data.set_max_level(tag, level);
}

void Logger::log(log::Level const level, CString const message) const { log::log_message(level, tag.c_str(), message); }
} // namespace bave
