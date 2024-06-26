#include <bave/core/hash_combine.hpp>
#include <bave/graphics/detail/pipeline_cache.hpp>
#include <bave/graphics/geometry.hpp>
#include <bave/logger.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <cstddef>
#include <map>

namespace bave::detail {
namespace {
struct PipelineShaderLayout {
	std::vector<vk::UniqueDescriptorSetLayout> descriptor_set_layouts{};
	std::vector<vk::DescriptorSetLayout> descriptor_set_layouts_view{};

	static auto make(vk::Device device) -> PipelineShaderLayout {
		auto ordered_set_layouts = std::map<std::uint32_t, std::vector<vk::DescriptorSetLayoutBinding>>{};

		auto& set_0 = ordered_set_layouts[0];
		set_0.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1);
		set_0.emplace_back(1, vk::DescriptorType::eStorageBuffer, 1);

		auto& textures_set = ordered_set_layouts[1];
		for (std::uint32_t binding = 0; binding < SetLayout::max_textures_v; ++binding) {
			textures_set.emplace_back(binding, vk::DescriptorType::eCombinedImageSampler, 1);
		}

		auto& buffers_set = ordered_set_layouts[2];
		buffers_set.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1);
		buffers_set.emplace_back(1, vk::DescriptorType::eStorageBuffer, 1);

		for (auto& [_, bindings] : ordered_set_layouts) {
			for (auto& binding : bindings) { binding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment; }
		}

		auto ret = PipelineShaderLayout{};
		for (auto const& [set, bindings] : ordered_set_layouts) {
			auto dslci = vk::DescriptorSetLayoutCreateInfo{};
			dslci.bindingCount = static_cast<std::uint32_t>(bindings.size());
			dslci.pBindings = bindings.data();
			ret.descriptor_set_layouts.push_back(device.createDescriptorSetLayoutUnique(dslci));
			ret.descriptor_set_layouts_view.push_back(*ret.descriptor_set_layouts.back());
		}
		return ret;
	}
};
} // namespace

PipelineCache::Key::Key(Program shader, State state)
	: shader(shader), state(state), cached_hash(make_combined_hash(shader.vertex, shader.fragment, state.topology, state.polygon_mode)) {}

PipelineCache::PipelineCache(vk::RenderPass render_pass, NotNull<RenderDevice*> render_device, NotNull<DataStore const*> data_store)
	: m_shader_cache(render_device->get_device(), data_store), m_descriptor_cache(render_device), m_render_pass(render_pass),
	  m_samples(render_device->get_sample_count()) {
	auto pipeline_shader_layout = PipelineShaderLayout::make(render_device->get_device());

	m_descriptor_set_layouts = std::move(pipeline_shader_layout).descriptor_set_layouts;
	m_descriptor_set_layouts_view = std::move(pipeline_shader_layout.descriptor_set_layouts_view);

	auto plci = vk::PipelineLayoutCreateInfo{};
	plci.setLayoutCount = static_cast<std::uint32_t>(m_descriptor_set_layouts_view.size());
	plci.pSetLayouts = m_descriptor_set_layouts_view.data();
	m_pipeline_layout = render_device->get_device().createPipelineLayoutUnique(plci);

	m_vertex_layout.bindings = {
		vk::VertexInputBindingDescription{0, sizeof(Vertex)},
	};
	m_vertex_layout.attributes = {
		vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)},
		vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
		vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, rgba)},
	};
}

auto PipelineCache::load_pipeline(Program shader, State state) -> vk::Pipeline {
	if (!shader.vertex || !shader.fragment) {
		m_log.warn("null vertex/fragment shader");
		return {};
	}

	auto const key = Key{shader, state};
	auto itr = m_pipelines.find(key);
	if (itr == m_pipelines.end()) {
		auto ret = build(key);
		if (!ret) { return {}; }
		auto const [inserted, _] = m_pipelines.insert_or_assign(key, std::move(ret));
		itr = inserted;

		m_log.debug("new Vulkan Pipeline created '{}' (total: {})", key.hash(), m_pipelines.size());
	}
	assert(itr != m_pipelines.end());
	return *itr->second;
}

void PipelineCache::clear_loaded() {
	auto const pc = pipeline_count();
	auto const sc = shader_count();
	if (pc == 0 && sc == 0) { return; }
	m_shader_cache.get_device().waitIdle();
	m_pipelines.clear();
	m_shader_cache.clear();
	m_log.info("{} Vulkan Pipeline(s) and {} Shader Module(s) destroyed", pc, sc);
}

auto PipelineCache::build(Key const& key) -> vk::UniquePipeline {
	auto shader_stages = std::array<vk::PipelineShaderStageCreateInfo, 2>{};
	shader_stages[0].stage = vk::ShaderStageFlagBits::eVertex;
	shader_stages[1].stage = vk::ShaderStageFlagBits::eFragment;
	shader_stages[0].pName = shader_stages[1].pName = "main";

	shader_stages[0].module = key.shader.vertex;
	shader_stages[1].module = key.shader.fragment;
	assert(shader_stages[0].module && shader_stages[1].module);

	auto pvisci = vk::PipelineVertexInputStateCreateInfo{};
	pvisci.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(m_vertex_layout.attributes.size());
	pvisci.pVertexAttributeDescriptions = m_vertex_layout.attributes.data();
	pvisci.vertexBindingDescriptionCount = static_cast<std::uint32_t>(m_vertex_layout.bindings.size());
	pvisci.pVertexBindingDescriptions = m_vertex_layout.bindings.data();

	auto gpci = vk::GraphicsPipelineCreateInfo{};
	gpci.pVertexInputState = &pvisci;
	gpci.stageCount = static_cast<std::uint32_t>(shader_stages.size());
	gpci.pStages = shader_stages.data();

	auto prsci = vk::PipelineRasterizationStateCreateInfo{};
	prsci.polygonMode = key.state.polygon_mode;
	prsci.cullMode = vk::CullModeFlagBits::eNone;
	gpci.pRasterizationState = &prsci;

	auto const piasci = vk::PipelineInputAssemblyStateCreateInfo{{}, key.state.topology};
	gpci.pInputAssemblyState = &piasci;

	auto pcbas = vk::PipelineColorBlendAttachmentState{};
	using CCF = vk::ColorComponentFlagBits;
	pcbas.colorWriteMask = CCF::eR | CCF::eG | CCF::eB | CCF::eA;
	pcbas.blendEnable = 1;
	pcbas.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	pcbas.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	pcbas.colorBlendOp = vk::BlendOp::eAdd;
	pcbas.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	pcbas.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	pcbas.alphaBlendOp = vk::BlendOp::eAdd;
	auto pcbsci = vk::PipelineColorBlendStateCreateInfo();
	pcbsci.attachmentCount = 1;
	pcbsci.pAttachments = &pcbas;
	gpci.pColorBlendState = &pcbsci;

	auto pdsci = vk::PipelineDynamicStateCreateInfo();
	auto const pdscis = std::array{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::eLineWidth,
	};
	pdsci = vk::PipelineDynamicStateCreateInfo({}, static_cast<std::uint32_t>(pdscis.size()), pdscis.data());
	gpci.pDynamicState = &pdsci;

	auto pvsci = vk::PipelineViewportStateCreateInfo({}, 1, {}, 1);
	gpci.pViewportState = &pvsci;

	auto pmsci = vk::PipelineMultisampleStateCreateInfo{};
	pmsci.rasterizationSamples = m_samples;
	pmsci.sampleShadingEnable = vk::False;
	gpci.pMultisampleState = &pmsci;

	gpci.renderPass = m_render_pass;
	gpci.layout = *m_pipeline_layout;

	auto ret = vk::Pipeline{};
	if (m_shader_cache.get_device().createGraphicsPipelines({}, 1, &gpci, {}, &ret) != vk::Result::eSuccess) { return {}; }

	return vk::UniquePipeline{ret, m_shader_cache.get_device()};
}
} // namespace bave::detail
