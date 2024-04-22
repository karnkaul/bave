#pragma once
#include <bave/core/c_string.hpp>
#include <bave/core/polymorphic.hpp>

namespace bave {
class DataLoader : public Polymorphic {
  public:
	[[nodiscard]] virtual auto get_name() const -> std::string_view = 0;

	[[nodiscard]] virtual auto exists(CString path) const -> bool = 0;
	[[nodiscard]] virtual auto read_bytes(std::vector<std::byte>& out, CString path) const -> bool = 0;

	[[nodiscard]] virtual auto read_string(std::string& out, CString path) const -> bool {
		auto bytes = std::vector<std::byte>{};
		if (!read_bytes(bytes, path)) { return false; }
		out = std::string{reinterpret_cast<char const*>(bytes.data()), bytes.size()}; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		return true;
	}

	virtual auto set_mount_point(std::string_view directory) -> bool = 0;
};
} // namespace bave
