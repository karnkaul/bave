#pragma once
#include <compare>
#include <optional>
#include <string>

namespace bave {
/// \brief Semantic version.
struct Version {
	/// \brief Major version.
	int major{};
	/// \brief Minor version.
	int minor{};
	/// \brief Patch version.
	int patch{};

	auto operator<=>(Version const&) const = default;

	/// \brief Parse version text.
	/// \param text string to parse.
	/// \returns Version if parsed successfully, else std::nullopt.
	static auto from(std::string text) -> std::optional<Version>;
};

/// \brief Serialize Version to string.
/// \param version Version to serialize.
/// \returns Serialized version string.
[[nodiscard]] auto to_string(Version const& version) -> std::string;
} // namespace bave
