#pragma once
#include <bave/data_loader.hpp>
#include <bave/logger.hpp>

namespace bave {
class FileLoader : public IDataLoader {
  public:
	explicit FileLoader(std::string_view mount_point);

	[[nodiscard]] auto make_full_path(std::string_view uri) const -> std::string;

  private:
	[[nodiscard]] auto exists(std::string_view uri) const -> bool final;
	auto read_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool final;
	auto read_string(std::string& out, std::string_view uri) const -> bool final;

	Logger m_log{"FileLoader"};
	std::string m_prefix{};
};
} // namespace bave
