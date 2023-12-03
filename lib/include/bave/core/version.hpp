#pragma once
#include <compare>
#include <optional>
#include <string>

namespace bave {
struct Version {
	int major{};
	int minor{};
	int patch{};

	auto operator<=>(Version const&) const = default;

	static auto from(std::string text) -> std::optional<Version>;
};

[[nodiscard]] auto to_string(Version const& version) -> std::string;
} // namespace bave
