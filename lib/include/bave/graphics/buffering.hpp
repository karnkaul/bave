#pragma once
#include <bave/core/wrap.hpp>
#include <array>
#include <cstdint>

namespace bave {
constexpr size_t buffering_v{2};

template <typename Type>
using Buffered = std::array<Type, buffering_v>;

struct FrameIndex {
	size_t value{};

	constexpr auto increment() -> void { value = increment_wrapped(value, buffering_v); }

	constexpr operator size_t() const { return value; }
};

template <typename BufferedT, typename MakeT>
auto fill_buffered(BufferedT& out, MakeT make) -> void {
	for (auto& type : out) { type = make(); }
}

template <typename Type, typename MakeT>
auto make_buffered(MakeT make) -> Buffered<Type> {
	auto ret = Buffered<Type>{};
	fill_buffered(ret, std::move(make));
	return ret;
}
} // namespace bave
