#pragma once
#include <array>

namespace bave::detail {
constexpr std::size_t buffering_v{2};

template <typename Type>
using Buffered = std::array<Type, buffering_v>;

struct FrameIndex {
	std::size_t value{};

	constexpr auto increment() -> void { value = (value + 1) % buffering_v; }

	constexpr operator std::size_t() const { return value; }
};

template <typename BufferedT, typename FactoryT>
auto fill_buffered(BufferedT& out, FactoryT factory) -> void {
	for (auto& type : out) { type = factory(); }
}

template <typename Type, typename FactoryT, std::size_t... I>
auto make_buffered_impl(FactoryT factory, std::index_sequence<I...> /*is*/) {
	struct BufferedFactory {
		FactoryT factory;

		constexpr auto operator[](std::size_t /*index*/) const -> FactoryT const& { return factory; }
	};

	auto const bf = BufferedFactory{.factory = std::move(factory)};
	return Buffered<Type>{bf[I]()...};
}

template <typename Type, typename FactoryT>
auto make_buffered(FactoryT factory) -> Buffered<Type> {
	return make_buffered_impl<Type>(std::move(factory), std::make_index_sequence<buffering_v>());
}
} // namespace bave::detail
