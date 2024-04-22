#pragma once
#include <bave/core/c_string.hpp>
#include <bave/data_loader.hpp>
#include <bave/logger.hpp>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace dj {
class Json;
}

namespace bave {
class DataStore {
  public:
	[[nodiscard]] static auto as_string_view(std::span<std::byte const> bytes) -> std::string_view;

	void add_loader(std::unique_ptr<DataLoader> loader, int priority = 0);

	[[nodiscard]] auto exists(std::string_view uri) const -> bool;

	[[nodiscard]] auto read_bytes(std::string_view uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto read_string(std::string_view uri) const -> std::string;
	[[nodiscard]] auto read_json(std::string_view uri) const -> dj::Json;

	[[nodiscard]] auto to_spir_v(std::string_view glsl) const -> std::string;

  private:
	struct Entry {
		std::unique_ptr<DataLoader> loader{};
		int priority{};
	};

	Logger m_log{"DataStore"};
	std::vector<Entry> m_loaders{};
};
} // namespace bave
