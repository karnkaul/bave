#pragma once
#include <bave/core/not_null.hpp>
#include <bave/core/string_hash.hpp>
#include <bave/data_store.hpp>
#include <bave/logger.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace bave {
class ShaderCache {
  public:
	explicit ShaderCache(vk::Device device, NotNull<DataStore const*> data_store) : m_device(device), m_data_store(data_store) {}

	[[nodiscard]] auto get_data_store() const -> DataStore const& { return *m_data_store; }
	[[nodiscard]] auto get_device() const -> vk::Device { return m_device; }

	[[nodiscard]] auto load(std::string_view uri) -> vk::ShaderModule;

	[[nodiscard]] auto shader_count() const -> std::size_t { return m_modules.size(); }

  private:
	vk::Device m_device;
	NotNull<DataStore const*> m_data_store;
	std::unordered_map<std::string, vk::UniqueShaderModule, StringHash, std::equal_to<>> m_modules{};
	Logger m_log{"ShaderCache"};
};
} // namespace bave
