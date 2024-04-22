#pragma once
#include <bave/data_store.hpp>

namespace bave::detail {
struct DataStoreProvider : Polymorphic {
	[[nodiscard]] virtual auto get_data_store() const -> DataStore const& = 0;
};
} // namespace bave::detail
