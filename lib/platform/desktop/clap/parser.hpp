#pragma once
#include <bave/core/error.hpp>
#include <platform/desktop/clap/clap.hpp>
#include <span>

namespace bave::clap {
struct Option {
	std::string key{};
	char letter{};
	std::string description{};
	detail::Argument argument{};
	bool* out_was_passed{};

	void parse_argument(std::string_view value) const;
};

struct OptionSpec {
	std::vector<Option> positional{};
	std::vector<Option> options{};
	Option unmatched{};
};

/*
1: -abc=foo
	^^^ ~~~
2: -abc foo
	^^^ ~~~
3: -abcfoo
	^^^~~~
4: --long=foo
	 ^    ~~~
5: --long foo
	 ^    ~~~
*/

struct Parser {
	static auto invalid_option(std::string_view option) -> Error;
	static auto unrecognized_argument(std::string_view argument) -> Error;
	static auto unexpected_argument(std::string_view option) -> Error;
	static auto argument_required(std::string_view option) -> Error;
	static auto invalid_value(std::string_view option, std::string_view value) -> Error;

	std::span<char const* const> input{};
	OptionSpec spec{};

	std::string_view current{};
	bool force_positional{};
	std::size_t position{};

	auto parse_next() -> bool;

	void argument();
	void option();

	void long_option();
	void short_options();

	[[nodiscard]] auto at_end() const -> bool { return input.empty(); }
	[[nodiscard]] auto peek() const -> std::string_view { return at_end() ? std::string_view{} : input[0]; }
	auto advance() -> bool;
};
} // namespace bave::clap
