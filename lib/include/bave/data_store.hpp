#pragma once
#include <bave/core/c_string.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/data_loader.hpp>
#include <bave/logger.hpp>
#include <cstddef>
#include <span>
#include <vector>

namespace dj {
class Json;
}

namespace bave {
class DataStore : public Polymorphic {
  public:
	explicit DataStore(std::string tag = "DataStore") : m_log{std::move(tag)} {}

	[[nodiscard]] static auto as_string_view(std::span<std::byte const> bytes) -> std::string_view;

	[[nodiscard]] auto exists(std::string_view uri) const -> bool { return !uri.empty() && do_exists(make_full_path(uri).c_str()); }

	[[nodiscard]] auto read_bytes(std::string_view uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto read_string(std::string_view uri) const -> std::string;
	[[nodiscard]] auto read_json(std::string_view uri) const -> dj::Json;

	[[nodiscard]] auto get_mount_point() const -> std::string_view { return m_prefix; }
	auto set_mount_point(std::string_view directory) -> bool { return do_set_mount_point(directory); }

	[[nodiscard]] auto make_full_path(std::string_view uri) const -> std::string;
	[[nodiscard]] auto make_uri(std::string_view full_path) const -> std::string;

	[[nodiscard]] auto to_spir_v(std::string_view glsl) const -> std::string;

  protected:
	std::string m_prefix{};
	Logger m_log;

  private:
	[[nodiscard]] virtual auto do_exists(CString /*path*/) const -> bool { return false; }
	[[nodiscard]] virtual auto do_read_bytes(std::vector<std::byte>& /*out*/, CString /*path*/) const -> bool { return false; }
	[[nodiscard]] virtual auto do_read_string(std::string& out, CString path) const -> bool;
	virtual auto do_set_mount_point(std::string_view /*directory*/) -> bool { return false; }
};
} // namespace bave
