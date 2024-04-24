#include <bave/io/data_loader_builder.hpp>
#include <bave/io/file_io.hpp>
#include <bave/io/file_loader.hpp>
#include <bave/io/zip_io.hpp>
#include <bave/io/zip_loader.hpp>
#include <bave/logger.hpp>
#include <src/io/zip_io_reinit.hpp>
#include <filesystem>
#include <span>

namespace bave {
namespace fs = std::filesystem;

DataLoaderBuilder::DataLoaderBuilder(int argc, char const* const* argv) {
	auto args = std::span{argv, static_cast<std::size_t>(argc)};
	if (args.empty()) { return; }

	m_exe_dir = fs::absolute({args.front()}).parent_path().make_preferred().generic_string();
	zip::reinit(args.front());
}

auto DataLoaderBuilder::upfind(std::string_view const patterns) const -> std::string { return file::upfind(get_exe_dir(), patterns); }

auto DataLoaderBuilder::upfind_parent(std::string_view const filename) const -> std::string { return file::upfind_parent(get_exe_dir(), filename); }

auto DataLoaderBuilder::add_zip(std::string uri) -> DataLoaderBuilder& {
	if (uri.empty()) { return *this; }
	m_entries.push_back(Entry{.input = std::move(uri), .type = Type::eZip});
	return *this;
}

auto DataLoaderBuilder::add_dir(std::string patterns) -> DataLoaderBuilder& {
	if (patterns.empty()) { return *this; }
	m_entries.push_back(Entry{.input = std::move(patterns), .type = Type::eDir});
	return *this;
}

auto DataLoaderBuilder::build() const -> std::unique_ptr<IDataLoader> {
	auto ret = std::unique_ptr<IDataLoader>{};
	for (auto const& entry : m_entries) {
		switch (entry.type) {
		case Type::eDir: ret = build_dir(entry.input); break;
		case Type::eZip: ret = build_zip(entry.input); break;
		default: break;
		}
		if (ret) { return ret; }
	}

	// fallback: FileLoader using pwd.
	auto mount_point = fs::current_path().generic_string();
	m_log.info("using FileLoader with working directory mounted: '{}'", mount_point);
	return std::make_unique<FileLoader>(std::move(mount_point));
}

auto DataLoaderBuilder::build_dir(std::string_view const patterns) const -> std::unique_ptr<IDataLoader> {
	auto mount_point = file::upfind_dir(m_exe_dir, patterns);
	if (mount_point.empty()) {
		m_log.warn("failed to mount directory: '{}'", mount_point);
		return {};
	}

	m_log.info("using FileLoader with mounted path: '{}'", mount_point);
	return std::make_unique<FileLoader>(std::move(mount_point));
}

auto DataLoaderBuilder::build_zip(std::string_view const uri) const -> std::unique_ptr<IDataLoader> {
	auto zip_path = file::upfind(m_exe_dir, uri);
	if (zip_path.empty()) {
		m_log.warn("failed to find ZIP: '{}'", zip_path);
		return {};
	}

	auto zip_bytes = std::vector<std::byte>{};
	if (!file::read_bytes(zip_bytes, zip_path.c_str())) {
		m_log.warn("failed to read ZIP: '{}'", zip_path);
		return {};
	}

	if (!zip::mount(std::string{uri}, std::move(zip_bytes))) {
		m_log.warn("failed to mount ZIP: '{}'", zip_path);
		return {};
	}

	m_log.info("using ZipLoader with mounted ZIP: '{}'", zip_path);
	return std::make_unique<ZipLoader>();
}
} // namespace bave
