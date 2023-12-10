#pragma once
#include <bave/core/c_string.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/logger.hpp>
#include <djson/json.hpp>
#include <cstddef>
#include <span>
#include <vector>

namespace bave {
class DataStore : public Polymorphic {
  public:
	explicit DataStore(std::string tag = "DataStore") : m_log(std::move(tag)) {}

	[[nodiscard]] static auto as_string_view(std::span<std::byte const> bytes) -> std::string_view;

	[[nodiscard]] auto exists(std::string_view uri) const -> bool { return do_exists(make_full_path(uri).c_str()); }

	[[nodiscard]] auto read_bytes(std::string_view uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto read_string(std::string_view uri) const -> std::string;
	[[nodiscard]] auto read_json(std::string_view uri) const -> dj::Json;

	[[nodiscard]] auto get_prefix() const -> std::string_view { return m_prefix; }
	[[nodiscard]] auto make_full_path(std::string_view uri) const -> std::string;

	[[nodiscard]] auto to_spir_v(std::string_view glsl, bool try_compile = true) const -> std::string;

  protected:
	std::string m_prefix{};
	Logger m_log;

  private:
	[[nodiscard]] virtual auto do_exists(CString path) const -> bool = 0;
	[[nodiscard]] virtual auto do_read_bytes(std::vector<std::byte>& out, CString path) const -> bool = 0;
	[[nodiscard]] virtual auto do_read_string(std::string& out, CString path) const -> bool;
};
} // namespace bave
