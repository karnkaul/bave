#include <bave/graphics/detail/shader_cache.hpp>
#include <bave/logger.hpp>

namespace bave::detail {
auto ShaderCache::load(std::string_view const uri) -> vk::ShaderModule {
	if (uri.empty()) { return {}; }

	if (auto it = m_modules.find(uri); it != m_modules.end()) { return *it->second; }

	auto const spir_v_uri = get_data_store().to_spir_v(uri);
	if (spir_v_uri.empty()) { return {}; }
	auto const spir_v_bytes = get_data_store().read_bytes(spir_v_uri);
	if (spir_v_bytes.empty()) { return {}; }
	auto const smci = vk::ShaderModuleCreateInfo{
		{},
		spir_v_bytes.size(),
		reinterpret_cast<std::uint32_t const*>(spir_v_bytes.data()), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
	};
	auto shader_module = m_device.createShaderModuleUnique(smci);
	if (!shader_module) {
		m_log.error("failed to load shader: '{}'", uri);
		return {};
	}

	auto [it, _] = m_modules.insert_or_assign(std::string{uri}, std::move(shader_module));
	m_log.debug("new Vulkan Shader Module created '{}' (total: {})", uri, m_modules.size());

	return *it->second;
}
} // namespace bave::detail
