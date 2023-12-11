#include <bave/graphics/frame_renderer.hpp>
#include <bave/graphics/shader.hpp>
#include <glm/gtx/transform.hpp>

namespace bave {
namespace {
struct Std140View {
	glm::mat4 view;
	glm::mat4 projection;
};
} // namespace

Shader::Shader(NotNull<FrameRenderer const*> frame_renderer, vk::ShaderModule vert, vk::ShaderModule frag)
	: m_frame_renderer(frame_renderer), m_vert(vert), m_frag(frag) {
	m_scissor.extent = m_frame_renderer->get_backbuffer_extent();
	glm::vec2 const viewport = glm::uvec2{m_scissor.extent.width, m_scissor.extent.height};
	m_viewport = vk::Viewport{0.0f, viewport.y, viewport.x, -viewport.y};

	auto descriptor_set = get_descriptor_set(1);
	auto const [image_view, sampler] = frame_renderer->get_white_texture().combined_image_sampler();
	auto const dii = vk::DescriptorImageInfo{sampler, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
	auto wdss = std::array<vk::WriteDescriptorSet, static_cast<std::size_t>(PipelineCache::max_textures_v)>{};
	for (std::uint32_t binding = 0; binding < PipelineCache::max_textures_v; ++binding) {
		auto& wds = wdss.at(binding);
		wds = vk::WriteDescriptorSet{descriptor_set, binding, 0, 1, vk::DescriptorType::eCombinedImageSampler};
		wds.pImageInfo = &dii;
	}
	frame_renderer->get_render_device().get_device().updateDescriptorSets(wdss, {});
}

auto Shader::update(std::uint32_t set, std::uint32_t binding, RenderBuffer const& buffer) -> bool {
	auto descriptor_set = get_descriptor_set(set);
	if (!descriptor_set) { return false; }

	auto const descriptor_type = [usage = buffer.get_usage()] {
		if (usage & vk::BufferUsageFlagBits::eStorageBuffer) { return vk::DescriptorType::eStorageBuffer; }
		return vk::DescriptorType::eUniformBuffer;
	}();
	auto wds = vk::WriteDescriptorSet{descriptor_set, binding, 0, 1, descriptor_type};
	auto const dbi = vk::DescriptorBufferInfo{buffer.get_buffer(), {}, buffer.get_size()};
	wds.pBufferInfo = &dbi;

	m_frame_renderer->get_render_device().get_device().updateDescriptorSets(wds, {});
	return true;
}

auto Shader::update(std::uint32_t set, std::uint32_t binding, ImageSampler const combined_image_sampler, std::uint32_t const index) -> bool {
	auto const& [image_view, sampler] = combined_image_sampler;
	if (!image_view || !sampler) { return false; }
	auto descriptor_set = get_descriptor_set(set);
	if (!descriptor_set) { return false; }

	auto wds = vk::WriteDescriptorSet{descriptor_set, binding, index, 1, vk::DescriptorType::eCombinedImageSampler};
	auto const dii = vk::DescriptorImageInfo{sampler, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
	wds.pImageInfo = &dii;

	m_frame_renderer->get_render_device().get_device().updateDescriptorSets(wds, {});
	return true;
}

void Shader::draw(vk::CommandBuffer command_buffer, Mesh const& mesh, std::span<RenderInstance::Baked const> instances) {
	if (mesh.get_vertex_count() == 0 || instances.empty()) { return; }

	auto& pipeline_cache = m_frame_renderer->get_pipeline_cache();
	auto const pipeline_state = PipelineCache::State{.line_width = line_width, .topology = topology, .polygon_mode = polygon_mode};
	auto pipeline = pipeline_cache.load_pipeline({.vertex = m_vert, .fragment = m_frag}, pipeline_state);
	if (!pipeline) {
		m_log.error("failed to load pipeline");
		return;
	}

	write_view_set();
	write_instances_set(instances);

	for (auto& [number, descriptor_set] : m_descriptor_sets) {
		if (!descriptor_set) { continue; }
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_cache.get_pipeline_layout(), number, descriptor_set, {});
		descriptor_set = vk::DescriptorSet{};
	}

	command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	command_buffer.setViewport(0, m_viewport);
	command_buffer.setScissor(0, m_scissor);
	mesh.draw(command_buffer, static_cast<std::uint32_t>(instances.size()));
}

auto Shader::get_descriptor_set(std::uint32_t set) -> vk::DescriptorSet {
	auto& pipeline_cache = m_frame_renderer->get_pipeline_cache();
	auto const set_layouts = pipeline_cache.get_descriptor_set_layouts();
	if (set >= set_layouts.size()) {
		m_log.error("invalid set: '{}'", set);
		return {};
	}

	auto& descriptor_set = m_descriptor_sets[set];
	if (!descriptor_set) { descriptor_set = pipeline_cache.get_descriptor_cache().allocate(set_layouts[set]); }
	return descriptor_set;
}

void Shader::write_view_set() {
	auto const view_extent = m_frame_renderer->get_backbuffer_extent();
	auto const uview_extent = glm::uvec2{view_extent.width, view_extent.height};
	auto const fview_extent = 0.5f * glm::fvec2{uview_extent};
	auto const view = Std140View{
		.view = m_frame_renderer->get_render_device().render_view.matrix(),
		.projection = glm::ortho(-fview_extent.x, fview_extent.x, -fview_extent.y, fview_extent.y, -100.0f, 100.0f),
	};
	auto& ubo = m_frame_renderer->get_render_device().get_scratch_buffer_cache().allocate(vk::BufferUsageFlagBits::eUniformBuffer);
	ubo.write(&view, sizeof(view));
	update(0, 0, ubo);
}

void Shader::write_instances_set(std::span<RenderInstance::Baked const> instances) {
	auto& ssbo = m_frame_renderer->get_render_device().get_scratch_buffer_cache().allocate(vk::BufferUsageFlagBits::eStorageBuffer);
	ssbo.write(instances.data(), instances.size_bytes());
	update(0, 1, ssbo);
}
} // namespace bave
