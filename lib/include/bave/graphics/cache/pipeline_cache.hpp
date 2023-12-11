#pragma once
#include <bave/graphics/cache/descriptor_cache.hpp>
#include <bave/graphics/cache/shader_cache.hpp>
#include <span>

namespace bave {
constexpr struct SetLayout {
	template <std::size_t Bindings>
	struct Set {
		std::uint32_t set{};
		std::array<vk::DescriptorType, Bindings> bindings{};
	};

	static constexpr std::uint32_t max_textures_v{4};

	static constexpr auto make_textures_set(std::uint32_t const set) -> Set<max_textures_v> {
		auto ret = Set<max_textures_v>{.set = set};
		for (auto& type : ret.bindings) { type = vk::DescriptorType::eCombinedImageSampler; }
		return ret;
	}

	Set<2> view_instances = Set<2>{
		.set = 0,
		.bindings = {vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eStorageBuffer},
	};

	Set<max_textures_v> textures = make_textures_set(1);

	Set<2> buffers = Set<2>{
		.set = 2,
		.bindings = {vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eStorageBuffer},
	};
} set_layout_v{};

class PipelineCache {
  public:
	struct State {
		float line_width{1.0f};
		vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
		vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};
	};

	struct Program {
		vk::ShaderModule vertex{};
		vk::ShaderModule fragment{};
	};

	explicit PipelineCache(vk::RenderPass render_pass, NotNull<RenderDevice*> render_device, NotNull<DataStore const*> data_store);

	[[nodiscard]] auto load_pipeline(Program shader, State state) -> vk::Pipeline;

	[[nodiscard]] auto get_shader_cache() const -> ShaderCache const& { return m_shader_cache; }
	[[nodiscard]] auto get_shader_cache() -> ShaderCache& { return m_shader_cache; }
	[[nodiscard]] auto get_descriptor_cache() const -> DescriptorCache const& { return m_descriptor_cache; }
	[[nodiscard]] auto get_descriptor_cache() -> DescriptorCache& { return m_descriptor_cache; }

	[[nodiscard]] auto get_pipeline_layout() const -> vk::PipelineLayout { return *m_pipeline_layout; }
	[[nodiscard]] auto get_descriptor_set_layouts() const -> std::span<vk::DescriptorSetLayout const> { return m_descriptor_set_layouts_view; }

	[[nodiscard]] auto shader_count() const -> std::size_t { return m_shader_cache.shader_count(); }
	[[nodiscard]] auto pipeline_count() const -> std::size_t { return m_pipelines.size(); }

  private:
	struct Key {
	  public:
		explicit Key(Program shader, State state);

		[[nodiscard]] auto hash() const -> std::size_t { return cached_hash; }

		auto operator==(Key const& rhs) const -> bool { return hash() == rhs.hash(); }

		Program shader{};
		State state{};
		std::size_t cached_hash{};
	};

	struct Hasher {
		auto operator()(Key const& key) const -> std::size_t { return key.hash(); }
	};

	struct {
		std::vector<vk::VertexInputAttributeDescription> attributes{};
		std::vector<vk::VertexInputBindingDescription> bindings{};
	} m_vertex_layout{};

	[[nodiscard]] auto build(Key const& key) -> vk::UniquePipeline;

	Logger m_log{"PipelineCache"};

	ShaderCache m_shader_cache;
	DescriptorCache m_descriptor_cache;
	vk::RenderPass m_render_pass{};
	std::unordered_map<Key, vk::UniquePipeline, Hasher> m_pipelines{};
	std::vector<vk::UniqueDescriptorSetLayout> m_descriptor_set_layouts{};
	std::vector<vk::DescriptorSetLayout> m_descriptor_set_layouts_view{};
	vk::UniquePipelineLayout m_pipeline_layout{};
};
} // namespace bave
