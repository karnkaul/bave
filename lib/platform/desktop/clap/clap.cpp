#include <fmt/format.h>
#include <platform/desktop/clap/clap.hpp>
#include <platform/desktop/clap/parser.hpp>
#include <cassert>
#include <filesystem>
#include <iostream>

namespace bave::clap {
namespace {
struct Spec {
	std::string app_name{};
	std::string description{};
	std::string footer{};
	std::string version{};
	OptionSpec options{};

	static auto make_option(std::string group, std::string description) -> Option {
		auto ret = Option{.description = std::move(description)};
		if (group.size() > 2 && group[1] == ',') {
			ret.letter = group[0];
			ret.key = group.substr(2);
		} else {
			ret.key = std::move(group);
			if (ret.key.size() == 1) { ret.letter = ret.key[0]; }
		}
		return ret;
	}
};

auto compute_width(Option const& option) -> std::uint32_t {
	auto ret = static_cast<std::uint32_t>(option.key.size()) + 2;
	if (option.argument) {
		ret += static_cast<std::uint32_t>(option.argument.usage.size());
		if (!option.argument.required) { ret += 2; }
	}
	return ret;
}

auto compute_max_width(std::span<Option const> options) -> std::uint32_t {
	auto ret = std::uint32_t{};
	for (auto const& option : options) { ret = std::max(ret, compute_width(option)); }
	return ret;
}

void print_letter_to(std::string& out, char const letter, bool print_blanks) {
	if (letter != 0) {
		fmt::format_to(std::back_inserter(out), "-{}, ", letter);
	} else if (print_blanks) {
		out += std::string(4, ' ');
	}
}

void print_key_to(std::string& out, Option const& option) {
	fmt::format_to(std::back_inserter(out), "--{}", option.key);
	if (option.argument) {
		auto const open = option.argument.required ? std::string_view{} : "[";
		auto const close = option.argument.required ? std::string_view{} : "]";
		fmt::format_to(std::back_inserter(out), "{}={}{}", open, option.argument.usage, close);
	}
}
} // namespace

struct Options::Impl {
	Spec spec{};

	void print_help() const {
		auto usage = fmt::format("Usage: {} ", spec.app_name);
		if (!spec.options.options.empty()) { fmt::format_to(std::back_inserter(usage), "[OPTION]... "); }
		for (auto const& positional : spec.options.positional) {
			assert(positional.argument);
			fmt::format_to(std::back_inserter(usage), "<{}> ", positional.argument.usage);
		}
		if (spec.options.unmatched.argument) { fmt::format_to(std::back_inserter(usage), "{}", spec.options.unmatched.argument.usage); }
		std::cout << fmt::format("{}\n", usage);

		std::cout << fmt::format("\n{}\n", spec.description);

		if (!spec.options.options.empty()) {
			std::cout << "\nOPTIONS\n\n";
			auto const max_width = compute_max_width(spec.options.options) + 12;
			for (auto const& option : spec.options.options) {
				auto text = std::string{"  "};
				print_letter_to(text, option.letter, true);
				print_key_to(text, option);

				text += std::string(static_cast<std::size_t>(max_width) - text.size(), ' ');
				fmt::format_to(std::back_inserter(text), "{}", option.description);
				std::cout << fmt::format("{}\n", text);
			}
		}

		if (!spec.footer.empty()) { std::cout << fmt::format("\n{}\n", spec.footer); }
	}
};

// NOLINTNEXTLINE
void Options::Deleter::operator()(Impl const* ptr) const { delete ptr; }

Options::Options(std::string app_name, std::string description, std::string version) : m_impl(new Impl{}) {
	m_impl->spec = {
		.app_name = std::move(app_name),
		.description = std::move(description),
		.version = std::move(version),
	};
}

auto Options::set_footer(std::string footer) -> Options& {
	m_impl->spec.footer = std::move(footer);
	return *this;
}

auto Options::flag(bool& out_flag, std::string group, std::string description) -> Options& {
	auto flag = Spec::make_option(std::move(group), std::move(description));
	flag.out_was_passed = &out_flag;
	if (!flag.key.empty()) { m_impl->spec.options.options.push_back(std::move(flag)); }
	return *this;
}

auto Options::bind_option(bool* out_was_passed, detail::Argument argument, std::string group, std::string description) -> Options& {
	auto option = Spec::make_option(std::move(group), std::move(description));
	if (option.key.empty()) { return *this; }
	option.out_was_passed = out_was_passed;
	if (argument.from_string) { option.argument = std::move(argument); }
	m_impl->spec.options.options.push_back(std::move(option));
	return *this;
}

auto Options::bind_positional(detail::FromString from_string, std::string key, std::string usage) -> Options& {
	auto positional = Option{.key = std::move(key)};
	if (positional.key.empty()) { return *this; }
	if (usage.empty()) { usage = positional.key; }
	positional.argument = detail::Argument{.usage = std::move(usage), .from_string = std::move(from_string)};
	m_impl->spec.options.positional.push_back(std::move(positional));
	return *this;
}

auto Options::bind_unmatched(detail::FromString from_string, std::string usage) -> Options& {
	m_impl->spec.options.unmatched.argument = detail::Argument{.usage = std::move(usage), .from_string = std::move(from_string)};
	return *this;
}

auto Options::parse(std::span<char const* const> args) -> Result {
	struct {
		bool help{};
		bool version{};
	} data{};
	auto parser = Parser{args, std::move(m_impl->spec.options)};
	auto option = Spec::make_option("help", "show help");
	option.out_was_passed = &data.help;
	parser.spec.options.push_back(std::move(option));
	option = Spec::make_option("version", "show version");
	option.out_was_passed = &data.version;
	parser.spec.options.push_back(std::move(option));

	try {
		while (parser.parse_next()) { ; }
		if (data.help || data.version) {
			if (data.help) {
				m_impl->spec.options = std::move(parser.spec);
				m_impl->print_help();
			} else {
				std::cout << fmt::format("{} version {}\n", m_impl->spec.app_name, m_impl->spec.version);
			}
			return Result::eExit;
		}
		if (parser.position < parser.spec.positional.size()) { throw Error{"missing {} operand", parser.spec.positional[parser.position].key}; }
	} catch (Error const& error) {
		std::cerr << fmt::format("{}: {}\nTry '{} --help' for more information.\n", m_impl->spec.app_name, error.what(), m_impl->spec.app_name);
		return Result::eParseError;
	}

	return Result::eContinue;
}

auto Options::parse(int argc, char const* const* argv, bool const parse_arg0) -> Result {
	auto span = std::span{argv, static_cast<std::size_t>(argc)};
	if (!parse_arg0 && !span.empty()) { span = span.subspan(1); }
	return parse(span);
}
} // namespace bave::clap

auto bave::clap::make_app_name(std::string_view const arg0) -> std::string { return std::filesystem::path{arg0}.stem().string(); }
