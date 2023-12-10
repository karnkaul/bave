#pragma once
#include <bave/core/not_null.hpp>
#include <bave/data_store.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eAndroid);

extern "C" {
struct android_app;
}

namespace bave {
class AndroidDataStore : public DataStore {
  public:
	[[nodiscard]] static auto find_super_dir(std::string_view base, std::string_view pattern) -> std::string;

	explicit AndroidDataStore(NotNull<android_app*> app) : m_app(app) {}

  private:
	[[nodiscard]] auto do_exists(CString path) const -> bool final;
	[[nodiscard]] auto do_read_bytes(std::vector<std::byte>& out, CString path) const -> bool final;
	[[nodiscard]] auto do_read_string(std::string& out, CString path) const -> bool final;

	NotNull<android_app*> m_app;
};
} // namespace bave
