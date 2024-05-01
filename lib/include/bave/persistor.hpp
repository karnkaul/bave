#pragma once
#include <bave/logger.hpp>
#include <span>
#include <string>
#include <vector>

namespace dj {
class Json;
}

namespace bave {
class App;

/// \brief Data reader/writer to platform's persistent storage.
class Persistor {
  public:
	/// \brief Construct an instance.
	/// \param app Const reference to active App instance.
	explicit Persistor(App const& app);

	/// \brief Obtain the full path of a given URI.
	/// \param uri URI to obtain full path of.
	/// \returns Full path as a string.
	[[nodiscard]] auto full_path(std::string_view uri) const -> std::string;

	/// \brief Check if a resource exists at a given URI.
	/// \param uri URI to check for.
	/// \returns true if a file exists.
	[[nodiscard]] auto exists(std::string_view uri) const -> bool;

	/// \brief Read bytes from a URI.
	/// \param uri URI to read from.
	/// \returns Bytes read as a vector, empty on failure.
	[[nodiscard]] auto read_bytes(std::string_view uri) const -> std::vector<std::byte>;
	/// \brief Read string from a URI.
	/// \param uri URI to read from.
	/// \returns Bytes read as a string, empty on failure.
	[[nodiscard]] auto read_string(std::string_view uri) const -> std::string;
	/// \brief Read JSON from a URI.
	/// \param uri URI to read from.
	/// \returns Bytes read as a JSON, empty on failure.
	[[nodiscard]] auto read_json(std::string_view uri) const -> dj::Json;

	/// \brief Write bytes to a URI.
	/// \param uri URI to write to.
	/// \param bytes Bytes to write.
	/// \returns true on success.
	// NOLINTNEXTLINE(modernize-use-nodiscard)
	auto write_bytes(std::string_view uri, std::span<std::byte const> bytes) const -> bool;
	/// \brief Write string to a URI.
	/// \param uri URI to write to.
	/// \param text String to write.
	/// \returns true on success.
	// NOLINTNEXTLINE(modernize-use-nodiscard)
	auto write_string(std::string_view uri, std::string_view text) const -> bool;
	/// \brief Write JSON to a URI.
	/// \param uri URI to write to.
	/// \param json JSON to write.
	/// \returns true on success.
	// NOLINTNEXTLINE(modernize-use-nodiscard)
	auto write_json(std::string_view uri, dj::Json const& json) const -> bool;

  private:
	template <typename Ret, typename F>
	auto do_read(std::string_view uri, F func) const -> Ret;

	template <typename T, typename F>
	auto do_write(std::string_view uri, T const& data, F func) const -> bool;

	Logger m_log{"Persistor"};
	std::string_view m_root_dir{};
};
} // namespace bave
