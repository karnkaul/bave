#include <bave/core/error.hpp>
#include <bave/graphics/renderer.hpp>
#include <bave/graphics/shader.hpp>
#include <glm/gtx/transform.hpp>

namespace bave {
namespace {
struct Std140ViewProjection {
	glm::mat4 view;
	glm::mat4 projection;
};
} // namespace

Shader::Shader(NotNull<Renderer const*> renderer, vk::ShaderModule vertex, vk::ShaderModule fragment) : m_renderer(renderer), m_vert(vertex), m_frag(fragment) {
	set_viewport_scissor();
	set_white_textures();
	set_blank_buffers();
}

auto Shader::update_texture(CombinedImageSampler const cis, std::uint32_t binding) -> bool {
	auto const& [image_view, sampler] = cis;
	if (!image_view || !sampler || binding >= SetLayout::max_textures_v) { return false; }
	auto descriptor_set = get_descriptor_set(set_layout_v.textures.set);
	if (!descriptor_set) { return false; }

	auto wds = vk::WriteDescriptorSet{descriptor_set, binding, 0, 1, vk::DescriptorType::eCombinedImageSampler};
	auto const dii = vk::DescriptorImageInfo{sampler, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
	wds.pImageInfo = &dii;

	m_renderer->get_render_device().get_device().updateDescriptorSets(wds, {});
	return true;
}

void Shader::update_textures(std::span<CombinedImageSampler const, SetLayout::max_textures_v> cis) {
	auto descriptor_set = get_descriptor_set(set_layout_v.textures.set);
	auto diis = std::array<vk::DescriptorImageInfo, SetLayout::max_textures_v>{};
	auto wdss = std::array<vk::WriteDescriptorSet, SetLayout::max_textures_v>{};
	auto size = std::size_t{};
	for (std::size_t binding = 0; binding < wdss.size(); ++binding) {
		auto const [image_view, sampler] = cis[binding];
		if (!image_view || !sampler) { continue; }
		auto const type = set_layout_v.textures.bindings.at(binding);
		auto& dii = diis.at(size);
		dii = {sampler, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
		auto& wds = wdss.at(size);
		wds = vk::WriteDescriptorSet{descriptor_set, static_cast<std::uint32_t>(binding), 0, 1, type};
		wds.pImageInfo = &dii;
		++size;
	}
	if (size == 0) { return; }

	m_renderer->get_render_device().get_device().updateDescriptorSets(std::span{wdss.data(), size}, {});
}

auto Shader::write_ubo(void const* data, vk::DeviceSize const size) -> bool {
	if (data == nullptr || size == 0) { return false; }
	return write<vk::BufferUsageFlagBits::eUniformBuffer>(get_descriptor_set(set_layout_v.buffers.set), 0, data, size);
}

auto Shader::write_ssbo(void const* data, vk::DeviceSize const size) -> bool {
	if (data == nullptr || size == 0) { return false; }
	return write<vk::BufferUsageFlagBits::eStorageBuffer>(get_descriptor_set(set_layout_v.buffers.set), 1, data, size);
}

void Shader::draw(vk::CommandBuffer command_buffer, Mesh const& mesh, std::span<RenderInstance::Baked const> instances) {
	if (mesh.get_vertex_count() == 0 || instances.empty()) { return; }

	auto& pipeline_cache = m_renderer->get_pipeline_cache();
	auto const pipeline_state = detail::PipelineCache::State{.line_width = line_width, .topology = topology, .polygon_mode = polygon_mode};
	auto pipeline = pipeline_cache.load_pipeline({.vertex = m_vert, .fragment = m_frag}, pipeline_state);
	if (!pipeline) {
		m_log.error("failed to load pipeline");
		return;
	}

	write_view_and_instances(instances);

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
	auto& pipeline_cache = m_renderer->get_pipeline_cache();
	auto const set_layouts = pipeline_cache.get_descriptor_set_layouts();
	if (set >= set_layouts.size()) {
		m_log.error("invalid set: '{}'", set);
		return {};
	}

	auto& descriptor_set = m_descriptor_sets[set];
	if (!descriptor_set) { descriptor_set = pipeline_cache.get_descriptor_cache().allocate(set_layouts[set]); }
	return descriptor_set;
}

auto Shader::allocate_scratch(vk::BufferUsageFlagBits const usage) const -> detail::RenderBuffer& {
	return m_renderer->get_render_device().get_scratch_buffer_cache().allocate(usage);
}

auto Shader::update(vk::DescriptorSet descriptor_set, std::uint32_t binding, detail::RenderBuffer const& buffer) -> bool {
	if (!descriptor_set) { throw Error{"Null DescriptorSet"}; }

	auto const descriptor_type = [usage = buffer.get_usage()] {
		if (usage & vk::BufferUsageFlagBits::eStorageBuffer) { return vk::DescriptorType::eStorageBuffer; }
		return vk::DescriptorType::eUniformBuffer;
	}();
	auto wds = vk::WriteDescriptorSet{descriptor_set, binding, 0, 1, descriptor_type};
	auto const dbi = vk::DescriptorBufferInfo{buffer, {}, buffer.get_size()};
	wds.pBufferInfo = &dbi;

	m_renderer->get_render_device().get_device().updateDescriptorSets(wds, {});
	return true;
}

void Shader::set_viewport_scissor() {
	m_scissor.extent = m_renderer->get_backbuffer_extent();
	glm::vec2 const viewport = glm::uvec2{m_scissor.extent.width, m_scissor.extent.height};
	m_viewport = vk::Viewport{0.0f, viewport.y, viewport.x, -viewport.y};
}

void Shader::write_view_and_instances(std::span<RenderInstance::Baked const> instances) {
	auto const& render_view = *m_renderer->render_view;
	auto const world_space = [&] {
		if (render_view.viewport) { return *render_view.viewport; }
		auto const view_extent = m_renderer->get_backbuffer_extent();
		auto const uview_extent = glm::uvec2{view_extent.width, view_extent.height};
		return glm::vec2{uview_extent};
	}();
	auto const proj_xy = 0.5f * world_space;
	auto const proj_z = render_view.z_plane;
	auto const view = Transform{
		.position = -render_view.transform.position,
		.rotation = -render_view.transform.rotation,
		.scale = render_view.transform.scale,
	};
	auto const view_projection = Std140ViewProjection{
		.view = view.matrix(),
		.projection = glm::ortho(-proj_xy.x, proj_xy.x, -proj_xy.y, proj_xy.y, proj_z.near, proj_z.far),
	};

	auto const descriptor_set = get_descriptor_set(set_layout_v.view_instances.set);
	write<vk::BufferUsageFlagBits::eUniformBuffer>(descriptor_set, 0, &view_projection, sizeof(view_projection));
	write<vk::BufferUsageFlagBits::eStorageBuffer>(descriptor_set, 1, instances.data(), instances.size_bytes());
}

void Shader::set_white_textures() {
	auto const combined_image_sampler = m_renderer->get_white_texture().combined_image_sampler();
	auto image_samplers = std::array<CombinedImageSampler, SetLayout::max_textures_v>{};
	for (auto& image_sampler : image_samplers) { image_sampler = combined_image_sampler; }
	update_textures(image_samplers);
}

void Shader::set_blank_buffers() {
	auto const descriptor_set = get_descriptor_set(set_layout_v.buffers.set);
	auto const& blank_ubo = m_renderer->get_render_device().get_scratch_buffer_cache().get_empty(vk::BufferUsageFlagBits::eUniformBuffer);
	auto const& blank_ssbo = m_renderer->get_render_device().get_scratch_buffer_cache().get_empty(vk::BufferUsageFlagBits::eStorageBuffer);
	update(descriptor_set, 0, blank_ubo);
	update(descriptor_set, 1, blank_ssbo);
}
} // namespace bave
