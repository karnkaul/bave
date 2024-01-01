#pragma once
#include <bave/data_store.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eDesktop);

namespace bave {
class DesktopDataStore : public DataStore {
	[[nodiscard]] auto do_exists(CString path) const -> bool final;
	[[nodiscard]] auto do_read_bytes(std::vector<std::byte>& out, CString path) const -> bool final;
	[[nodiscard]] auto do_read_string(std::string& out, CString path) const -> bool final;

  public:
	explicit DesktopDataStore(std::string data_dir);
};

[[nodiscard]] auto find_super_dir(std::string_view base, std::string_view patterns) -> std::string;
} // namespace bave
