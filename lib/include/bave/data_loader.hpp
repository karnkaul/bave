#pragma once
#include <bave/core/c_string.hpp>
#include <bave/core/polymorphic.hpp>

namespace bave {
class DataLoader : public Polymorphic {
  public:
	[[nodiscard]] virtual auto exists(std::string_view uri) const -> bool = 0;
	[[nodiscard]] virtual auto read_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool = 0;

	[[nodiscard]] virtual auto read_string(std::string& out, std::string_view uri) const -> bool {
		auto bytes = std::vector<std::byte>{};
		if (!read_bytes(bytes, uri)) { return false; }
		out = std::string{reinterpret_cast<char const*>(bytes.data()), bytes.size()}; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		return true;
	}
};
} // namespace bave
