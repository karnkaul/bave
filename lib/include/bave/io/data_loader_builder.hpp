#pragma once
#include <bave/data_loader.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eDesktop);

namespace bave {
/// \brief Concrete IDataLoader builder based on input searches for asset directories / ZIPs.
class DataLoaderBuilder {
  public:
	/// \brief Construct an instance.
	/// \param argc First argument to int main().
	/// \param argv Second argument to int main().
	explicit DataLoaderBuilder(int argc, char const* const* argv);

	/// \brief Get the path to the executable's parent directory.
	/// \returns Path to the executable's parent directory.
	[[nodiscard]] auto get_exe_dir() const -> std::string_view { return m_exe_dir; }

	/// \brief Find assets in a super directory of exe path / working directory.
	/// \param patterns Comma-separated list of file patterns to search for.
	/// \returns Path to directory if found, else empty string.
	[[nodiscard]] auto upfind(std::string_view patterns) const -> std::string;

	/// \brief Find the parent directory of a super directory containing a given file.
	/// \param filename Filename to search for.
	/// \returns Path to parent directory if found, else empty string.
	[[nodiscard]] auto upfind_parent(std::string_view filename) const -> std::string;

	/// \brief Add a directory search pattern.
	/// \param patterns Comma separated search patterns.
	/// \returns Self.
	auto add_dir(std::string patterns) -> DataLoaderBuilder&;
	/// \brief Add a zip file search.
	/// \param patterns ZIP filename to search for.
	/// \returns Self.
	auto add_zip(std::string uri) -> DataLoaderBuilder&;

	/// \brief Build a concrete IDataLoader based on first successful search.
	/// \returns Concrete IDataLoader.
	///
	/// If no searches pass, a FileLoader using the working directory is returned.
	[[nodiscard]] auto build() const -> std::unique_ptr<IDataLoader>;

  private:
	enum class Type : int { eDir, eZip };

	struct Entry {
		std::string input{};
		Type type{};
	};

	[[nodiscard]] auto build_dir(std::string_view patterns) const -> std::unique_ptr<IDataLoader>;
	[[nodiscard]] auto build_zip(std::string_view uri) const -> std::unique_ptr<IDataLoader>;

	Logger m_log{"DataLoaderBuilder"};

	std::string m_exe_dir{};
	std::vector<Entry> m_entries{};
};
} // namespace bave
