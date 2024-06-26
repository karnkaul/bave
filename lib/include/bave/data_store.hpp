#pragma once
#include <bave/data_loader.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <span>

namespace dj {
class Json;
}

namespace bave {
/// \brief Central data store, maintains an ordered list of `IDataLoader`s.
///
/// The stored loaders are iterated through for each API function, with the first
/// success being returned. If all loaders fail (or no loaders present), the call fails.
class DataStore {
  public:
	/// \brief View bytes as a string view.
	/// \param bytes Bytes to interpret.
	/// \returns string_view into the bytes.
	[[nodiscard]] static auto as_string_view(std::span<std::byte const> bytes) -> std::string_view;

	/// \brief Use a custom DataLoader.
	/// \param loader Custom DataLoader to use.
	void set_loader(std::unique_ptr<IDataLoader> loader);

	/// \brief Check if a resource exists at the given URI.
	/// \param uri URI of the resource.
	/// \returns true if a resource exists.
	[[nodiscard]] auto exists(std::string_view uri) const -> bool;

	/// \brief Read bytes from a given URI.
	/// \param uri URI to read from.
	/// \returns Vector of bytes, empty on failure.
	[[nodiscard]] auto read_bytes(std::string_view uri) const -> std::vector<std::byte>;
	/// \brief Read string from a given URI.
	/// \param uri URI to read from.
	/// \returns String, empty on failure.
	[[nodiscard]] auto read_string(std::string_view uri) const -> std::string;
	/// \brief Read JSON from a given URI.
	/// \param uri URI to read from.
	/// \returns JSON, null object on failure.
	[[nodiscard]] auto read_json(std::string_view uri) const -> dj::Json;

	/// \brief Convert a GLSL URI to SPIR-V.
	/// \param glsl GLSL URI.
	/// \returns SPIR-V URI if exists, else empty string.
	[[nodiscard]] auto to_spir_v(std::string_view glsl) const -> std::string;

  private:
	Logger m_log{"DataStore"};
	std::unique_ptr<IDataLoader> m_loader{};
};
} // namespace bave
