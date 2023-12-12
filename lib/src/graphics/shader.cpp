#include <bave/graphics/frame_renderer.hpp>
#include <bave/graphics/shader.hpp>
#include <glm/gtx/transform.hpp>

namespace bave {
namespace {
struct Std140ViewProjection {
	glm::mat4 view;
	glm::mat4 projection;
};
} // namespace

Shader::Shader(NotNull<FrameRenderer const*> frame_renderer, Program program, RenderView render_view)
	: m_frame_renderer(frame_renderer), m_vert(program.vertex), m_frag(program.fragment), m_render_view(render_view) {
	set_viewport_scissor();
	set_white_textures();
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

auto Shader::update(std::uint32_t set, std::uint32_t binding, ImageSampler const combined_image_sampler) -> bool {
	auto const& [image_view, sampler] = combined_image_sampler;
	if (!image_view || !sampler) { return false; }
	auto descriptor_set = get_descriptor_set(set);
	if (!descriptor_set) { return false; }

	auto wds = vk::WriteDescriptorSet{descriptor_set, binding, 0, 1, vk::DescriptorType::eCombinedImageSampler};
	auto const dii = vk::DescriptorImageInfo{sampler, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
	wds.pImageInfo = &dii;

	m_frame_renderer->get_render_device().get_device().updateDescriptorSets(wds, {});
	return true;
}

void Shader::update_textures(std::span<ImageSampler const, SetLayout::max_textures_v> combined_image_samplers) {
	auto descriptor_set = get_descriptor_set(set_layout_v.textures.set);
	auto diis = std::array<vk::DescriptorImageInfo, SetLayout::max_textures_v>{};
	auto wdss = std::array<vk::WriteDescriptorSet, SetLayout::max_textures_v>{};
	auto size = std::size_t{};
	for (std::size_t binding = 0; binding < wdss.size(); ++binding) {
		auto const [image_view, sampler] = combined_image_samplers[binding];
		if (!image_view || !sampler) { continue; }
		auto const type = set_layout_v.textures.bindings.at(binding);
		auto& dii = diis.at(size);
		dii = {sampler, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
		auto& wds = wdss.at(size);
		wds = vk::WriteDescriptorSet{descriptor_set, static_cast<std::uint32_t>(binding), 0, 1, type};
		wds.pImageInfo = &dii;
		++size;
	}
	m_frame_renderer->get_render_device().get_device().updateDescriptorSets(std::span{wdss.data(), size}, {});
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

	set_view_and_instances(instances);

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

void Shader::set_viewport_scissor() {
	m_scissor.extent = m_frame_renderer->get_backbuffer_extent();
	glm::vec2 const viewport = glm::uvec2{m_scissor.extent.width, m_scissor.extent.height};
	m_viewport = vk::Viewport{0.0f, viewport.y, viewport.x, -viewport.y};
}

void Shader::set_view_and_instances(std::span<RenderInstance::Baked const> instances) {
	auto const world_space = [&] {
		if (m_render_view.viewport) { return *m_render_view.viewport; }
		auto const view_extent = m_frame_renderer->get_backbuffer_extent();
		auto const uview_extent = glm::uvec2{view_extent.width, view_extent.height};
		return glm::vec2{uview_extent};
	}();
	auto const proj_xy = 0.5f * world_space;
	auto const proj_z = m_render_view.z_plane;
	auto const view = Transform{
		.position = -m_render_view.transform.position,
		.rotation = -m_render_view.transform.rotation,
		.scale = m_render_view.transform.scale,
	};
	auto const view_projection = Std140ViewProjection{
		.view = view.matrix(),
		.projection = glm::ortho(-proj_xy.x, proj_xy.x, -proj_xy.y, proj_xy.y, proj_z.near, proj_z.far),
	};
	auto& view_buffer = m_frame_renderer->get_render_device().get_scratch_buffer_cache().allocate(vk::BufferUsageFlagBits::eUniformBuffer);
	view_buffer.write(&view_projection, sizeof(view_projection));
	update(set_layout_v.view_instances.set, 0, view_buffer);

	auto& instances_buffer = m_frame_renderer->get_render_device().get_scratch_buffer_cache().allocate(vk::BufferUsageFlagBits::eStorageBuffer);
	instances_buffer.write(instances.data(), instances.size_bytes());
	update(set_layout_v.view_instances.set, 1, instances_buffer);
}

void Shader::set_white_textures() {
	auto const combined_image_sampler = m_frame_renderer->get_white_texture().combined_image_sampler();
	auto image_samplers = std::array<ImageSampler, SetLayout::max_textures_v>{};
	for (auto& image_sampler : image_samplers) { image_sampler = combined_image_sampler; }
	update_textures(image_samplers);
}
} // namespace bave
