#include <bave/graphics/frame_renderer.hpp>
#include <bave/graphics/shader.hpp>

namespace bave {
Shader::Shader(NotNull<FrameRenderer const*> frame_renderer, vk::ShaderModule vert, vk::ShaderModule frag)
	: m_frame_renderer(frame_renderer), m_vert(vert), m_frag(frag) {
	m_scissor.extent = m_frame_renderer->get_backbuffer_extent();
	glm::vec2 const viewport = glm::uvec2{m_scissor.extent.width, m_scissor.extent.height};
	m_viewport = vk::Viewport{0.0f, viewport.y, viewport.x, -viewport.y};
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

auto Shader::update(std::uint32_t set, std::uint32_t binding, RenderImage const& image, vk::Sampler const sampler) -> bool {
	auto descriptor_set = get_descriptor_set(set);
	if (!descriptor_set || !sampler) { return false; }

	auto wds = vk::WriteDescriptorSet{descriptor_set, binding, 0, 1, vk::DescriptorType::eCombinedImageSampler};
	auto const dii = vk::DescriptorImageInfo{sampler, image.get_image_view(), vk::ImageLayout::eShaderReadOnlyOptimal};
	wds.pImageInfo = &dii;

	m_frame_renderer->get_render_device().get_device().updateDescriptorSets(wds, {});
	return true;
}

void Shader::draw(vk::CommandBuffer command_buffer, std::uint32_t const vertex_count, std::uint32_t const instance_count) {
	if (vertex_count == 0 || instance_count == 0) { return; }

	auto& pipeline_cache = m_frame_renderer->get_pipeline_cache();
	auto pipeline = pipeline_cache.load_pipeline({.vertex = m_vert, .fragment = m_frag}, pipeline_state, polygon_mode);
	if (!pipeline) {
		m_log.error("failed to load pipeline");
		return;
	}

	for (auto& [number, descriptor_set] : m_descriptor_sets) {
		if (!descriptor_set) { continue; }
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_cache.get_pipeline_layout(), number, descriptor_set, {});
		descriptor_set = vk::DescriptorSet{};
	}

	command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	command_buffer.setViewport(0, m_viewport);
	command_buffer.setScissor(0, m_scissor);
	command_buffer.draw(vertex_count, instance_count, 0, 0);
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
} // namespace bave
