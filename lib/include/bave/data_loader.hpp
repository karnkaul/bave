#pragma once
#include <bave/core/c_string.hpp>
#include <bave/core/polymorphic.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace bave {
/// \brief Interface for loading bytes given a URI.
class DataLoader : public Polymorphic {
  public:
	/// \brief Check if a resource exists at the given URI.
	/// \param uri URI of the resource.
	/// \returns true if a resource exists.
	[[nodiscard]] virtual auto exists(std::string_view uri) const -> bool = 0;

	/// \brief Read bytes from a given URI.
	/// \param out Storage for bytes to be read.
	/// \param uri URI to read from.
	/// \returns true if successful.
	[[nodiscard]] virtual auto read_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool = 0;

	/// \brief Read string from a given URI.
	/// \param out Storage for string to be read.
	/// \param uri URI to read from.
	/// \returns true if successful.
	[[nodiscard]] virtual auto read_string(std::string& out, std::string_view uri) const -> bool {
		auto bytes = std::vector<std::byte>{};
		if (!read_bytes(bytes, uri)) { return false; }
		out = std::string{reinterpret_cast<char const*>(bytes.data()), bytes.size()}; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		return true;
	}
};
} // namespace bave
