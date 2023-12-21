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
}

auto Shader::update_texture(CombinedImageSampler const cis, std::uint32_t binding) -> bool {
	if (binding >= m_sets.cis.size()) { return false; }

	m_sets.cis.at(binding) = cis;
	return true;
}

void Shader::update_textures(std::span<CombinedImageSampler const, SetLayout::max_textures_v> cis) { std::copy(cis.begin(), cis.end(), m_sets.cis.begin()); }

auto Shader::write_ubo(void const* data, vk::DeviceSize const size) -> bool {
	if (data == nullptr || size == 0) { return false; }

	if (m_sets.ubo == nullptr) { m_sets.ubo = &allocate_scratch(vk::BufferUsageFlagBits::eUniformBuffer); }
	m_sets.ubo->write(data, size);
	return true;
}

auto Shader::write_ssbo(void const* data, vk::DeviceSize const size) -> bool {
	if (data == nullptr || size == 0) { return false; }

	if (m_sets.ssbo == nullptr) { m_sets.ssbo = &allocate_scratch(vk::BufferUsageFlagBits::eStorageBuffer); }
	m_sets.ssbo->write(data, size);
	return true;
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

	static_assert(set_layout_v.view_instances.set == 0);
	static_assert(set_layout_v.textures.set == 1);
	static_assert(set_layout_v.buffers.set == 2);

	auto& descriptor_cache = pipeline_cache.get_descriptor_cache();
	auto const descriptor_set_layouts = pipeline_cache.get_descriptor_set_layouts();
	auto descriptor_sets = std::array<vk::DescriptorSet, 3>{};
	descriptor_sets[0] = descriptor_cache.allocate(descriptor_set_layouts[0]);
	write_view_and_instances(descriptor_sets[0], instances);
	descriptor_sets[1] = descriptor_cache.allocate(descriptor_set_layouts[1]);
	update_textures(descriptor_sets[1]);
	descriptor_sets[2] = descriptor_cache.allocate(descriptor_set_layouts[2]);
	update_buffers(descriptor_sets[2]);

	command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_cache.get_pipeline_layout(), 0, descriptor_sets, {});

	command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	command_buffer.setViewport(0, m_viewport);
	command_buffer.setScissor(0, m_scissor);
	mesh.draw(command_buffer, static_cast<std::uint32_t>(instances.size()));

	m_sets = {}; // clear for next draw
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

void Shader::write_view_and_instances(vk::DescriptorSet descriptor_set, std::span<RenderInstance::Baked const> instances) {
	auto const& render_view = m_renderer->get_render_device().render_view;
	auto const proj_xy = 0.5f * render_view.viewport;
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

	write<vk::BufferUsageFlagBits::eUniformBuffer>(descriptor_set, 0, &view_projection, sizeof(view_projection));
	write<vk::BufferUsageFlagBits::eStorageBuffer>(descriptor_set, 1, instances.data(), instances.size_bytes());
}

void Shader::update_textures(vk::DescriptorSet descriptor_set) {
	auto diis = std::array<vk::DescriptorImageInfo, SetLayout::max_textures_v>{};
	auto wdss = std::array<vk::WriteDescriptorSet, SetLayout::max_textures_v>{};
	auto size = std::size_t{};
	for (std::size_t binding = 0; binding < wdss.size(); ++binding) {
		auto cis = m_sets.cis.at(binding);
		if (!cis.image_view || !cis.sampler) { cis = m_renderer->get_white_texture().combined_image_sampler(); }
		auto const type = set_layout_v.textures.bindings.at(binding);
		auto& dii = diis.at(size);
		dii = {cis.sampler, cis.image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
		auto& wds = wdss.at(size);
		wds = vk::WriteDescriptorSet{descriptor_set, static_cast<std::uint32_t>(binding), 0, 1, type};
		wds.pImageInfo = &dii;
		++size;
	}

	m_renderer->get_render_device().get_device().updateDescriptorSets(std::span{wdss.data(), size}, {});
}

void Shader::update_buffers(vk::DescriptorSet descriptor_set) {
	auto& scratch_buffer_cache = m_renderer->get_render_device().get_scratch_buffer_cache();
	auto const* ubo = m_sets.ubo;
	if (ubo == nullptr) { ubo = &scratch_buffer_cache.get_empty(vk::BufferUsageFlagBits::eUniformBuffer); }
	update(descriptor_set, 0, *ubo);
	auto const* ssbo = m_sets.ssbo;
	if (ssbo == nullptr) { ssbo = &scratch_buffer_cache.get_empty(vk::BufferUsageFlagBits::eStorageBuffer); }
	update(descriptor_set, 1, *ssbo);
}
} // namespace bave
