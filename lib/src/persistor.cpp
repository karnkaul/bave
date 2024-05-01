#include <bave/app.hpp>
#include <bave/io/file_io.hpp>
#include <bave/persistor.hpp>
#include <djson/json.hpp>
#include <filesystem>

namespace bave {
namespace fs = std::filesystem;

namespace {
template <typename F>
auto do_read(CString const path, F func) {
	return func(path);
}
} // namespace

Persistor::Persistor(App const& app) : m_root_dir(app.get_persistent_dir()) {}

auto Persistor::full_path(std::string_view const uri) const -> std::string { return (fs::path{m_root_dir} / uri).generic_string(); }

auto Persistor::exists(std::string_view const uri) const -> bool { return file::exists(full_path(uri).c_str()); }

auto Persistor::write_bytes(std::string_view const uri, std::span<std::byte const> bytes) const -> bool { return do_write(uri, bytes, &file::write_bytes); }

auto Persistor::write_string(std::string_view const uri, std::string_view const text) const -> bool { return do_write(uri, text, &file::write_string); }

auto Persistor::write_json(std::string_view const uri, dj::Json const& json) const -> bool { return write_string(uri, to_string(json)); }

auto Persistor::read_bytes(std::string_view const uri) const -> std::vector<std::byte> { return do_read<std::vector<std::byte>>(uri, &file::read_bytes); }

auto Persistor::read_string(std::string_view const uri) const -> std::string { return do_read<std::string>(uri, &file::read_string); }

auto Persistor::read_json(std::string_view const uri) const -> dj::Json {
	auto const ret = read_string(uri);
	if (ret.empty()) { return {}; }
	return dj::Json::parse(ret);
}

template <typename Ret, typename F>
auto Persistor::do_read(std::string_view const uri, F func) const -> Ret {
	auto ret = Ret{};
	if (uri.empty()) { return ret; }
	auto const path = full_path(uri);
	if (!func(ret, path.c_str())) {
		m_log.warn("failed to read from: '{}'", uri);
		return ret;
	}
	m_log.info("data read from: '{}'", uri);
	return ret;
}

template <typename T, typename F>
auto Persistor::do_write(std::string_view const uri, T const& data, F func) const -> bool {
	if (uri.empty()) { return false; }
	auto const path = full_path(uri);
	if (!func(path.c_str(), data)) {
		m_log.warn("failed to write to: '{}'", uri);
		return false;
	}
	m_log.info("data written to: '{}'", uri);
	return true;
}
} // namespace bave
