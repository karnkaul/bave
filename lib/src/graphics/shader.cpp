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

struct DescriptorBuffer {
	vk::Buffer buffer{};
	vk::DeviceSize offset{};
	vk::DeviceSize size{};
	vk::DescriptorType type{};

	DescriptorBuffer(detail::RenderBuffer const& buffer)
		: buffer(buffer.get_buffer()), size(buffer.get_size()),
		  type(buffer.get_usage() & vk::BufferUsageFlagBits::eStorageBuffer ? vk::DescriptorType::eStorageBuffer : vk::DescriptorType::eUniformBuffer) {}
};

template <typename Type>
struct ResourceBinding {
	Type resource{};
	std::uint32_t binding{};
};

template <typename Type, std::size_t Size>
struct DescriptorWrite {
	std::array<Type, Size> infos{};
	std::array<vk::WriteDescriptorSet, Size> writes{};
};

using BufferBinding = ResourceBinding<DescriptorBuffer>;
using ImageBinding = ResourceBinding<CombinedImageSampler>;

template <std::size_t Size>
using BufferWrite = DescriptorWrite<vk::DescriptorBufferInfo, Size>;

template <std::size_t Size>
using ImageWrite = DescriptorWrite<vk::DescriptorImageInfo, Size>;

template <std::size_t Size>
void make_buffer_write(BufferWrite<Size>& out, std::array<BufferBinding, Size> const& bindings, vk::DescriptorSet descriptor_set) {
	for (std::size_t i = 0; i < Size; ++i) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		auto const& resource = bindings[i].resource;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		out.infos[i] = vk::DescriptorBufferInfo{resource.buffer, resource.offset, resource.size};
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		out.writes[i] = vk::WriteDescriptorSet{descriptor_set, bindings[i].binding, 0, 1, resource.type};
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		out.writes[i].pBufferInfo = &out.infos[i];
	}
}

template <std::size_t Size>
void make_image_write(ImageWrite<Size>& out, std::array<ImageBinding, Size> const& bindings, vk::DescriptorSet descriptor_set) {
	for (std::size_t i = 0; i < Size; ++i) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		auto const cis = bindings[i].resource;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		out.infos[i] = vk::DescriptorImageInfo{cis.sampler, cis.image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		out.writes[i] = vk::WriteDescriptorSet{descriptor_set, bindings[i].binding, 0, 1, vk::DescriptorType::eCombinedImageSampler};
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		out.writes[i].pImageInfo = &out.infos[i];
	}
}

auto allocate_descriptor_sets(detail::PipelineCache& pipeline_cache) -> std::array<vk::DescriptorSet, 3> {
	auto const descriptor_set_layouts = pipeline_cache.get_descriptor_set_layouts();
	auto ret = std::array<vk::DescriptorSet, 3>{};
	for (std::size_t i = 0; i < ret.size(); ++i) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
		ret[i] = pipeline_cache.get_descriptor_cache().allocate(descriptor_set_layouts[i]);
	}
	return ret;
}

auto make_vpi_bindings(RenderDevice const& render_device, std::span<RenderInstance::Baked const> instances) {
	auto const& render_view = render_device.render_view;
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
	auto& vp_buf = render_device.get_buffer_cache().allocate(detail::BufferType::eUniform);
	vp_buf.write(&view_projection, sizeof(view_projection));
	auto& instances_buf = render_device.get_buffer_cache().allocate(detail::BufferType::eStorage);
	instances_buf.write(instances.data(), instances.size_bytes());
	return std::array{
		BufferBinding{.resource = vp_buf, .binding = 0},
		BufferBinding{.resource = instances_buf, .binding = 1},
	};
}

auto make_texture_bindings(std::span<CombinedImageSampler const, Shader::max_textures_v> textures, CombinedImageSampler const white) {
	auto ret = std::array<ImageBinding, Shader::max_textures_v>{};
	for (std::uint32_t i = 0; i < ret.size(); ++i) {
		auto cis = textures[i]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
		if (!cis.image_view || !cis.sampler) { cis = white; }
		ret[i] = ImageBinding{.resource = cis, .binding = i}; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
	}
	return ret;
}

auto make_buffer_bindings(detail::BufferCache& scratch_buffer_cache, Ptr<detail::RenderBuffer const> ubo, Ptr<detail::RenderBuffer const> ssbo) {
	auto const& custom_ubo = scratch_buffer_cache.or_empty(ubo, detail::BufferType::eUniform);
	auto const& custom_ssbo = scratch_buffer_cache.or_empty(ssbo, detail::BufferType::eStorage);
	return std::array{
		BufferBinding{.resource = custom_ubo, .binding = 0},
		BufferBinding{.resource = custom_ssbo, .binding = 1},
	};
}

[[nodiscard]] constexpr auto to_topology(Topology const in) {
	switch (in) {
	case Topology::eLineStrip: return vk::PrimitiveTopology::eLineStrip;
	default: return vk::PrimitiveTopology::eTriangleList;
	}
}
} // namespace

Shader::Shader(NotNull<Renderer const*> renderer, vk::ShaderModule vertex, vk::ShaderModule fragment) : m_renderer(renderer), m_vert(vertex), m_frag(fragment) {
	set_viewport();
}

auto Shader::update_texture(CombinedImageSampler const cis, std::uint32_t binding) -> bool {
	if (binding >= m_sets.cis.size()) { return false; }

	m_sets.cis.at(binding) = cis;
	return true;
}

void Shader::update_textures(std::span<CombinedImageSampler const, max_textures_v> cis) { std::copy(cis.begin(), cis.end(), m_sets.cis.begin()); }

auto Shader::write_ubo(void const* data, vk::DeviceSize const size) -> bool {
	if (data == nullptr || size == 0) { return false; }

	if (m_sets.ubo == nullptr) { m_sets.ubo = &allocate_scratch(detail::BufferType::eUniform); }
	m_sets.ubo->write(data, size);
	return true;
}

auto Shader::write_ssbo(void const* data, vk::DeviceSize const size) -> bool {
	if (data == nullptr || size == 0) { return false; }

	if (m_sets.ssbo == nullptr) { m_sets.ssbo = &allocate_scratch(detail::BufferType::eStorage); }
	m_sets.ssbo->write(data, size);
	return true;
}

void Shader::draw(RenderPrimitive const& primitive, std::span<RenderInstance::Baked const> instances) {
	auto const command_buffer = m_renderer->get_command_buffer();
	if (!command_buffer || primitive.bytes.empty() || instances.empty()) { return; }

	auto& pipeline_cache = m_renderer->get_pipeline_cache();
	auto const topology = to_topology(primitive.topology);
	auto const pipeline_state = detail::PipelineCache::State{.line_width = line_width, .topology = topology, .polygon_mode = polygon_mode};
	auto pipeline = pipeline_cache.load_pipeline({.vertex = m_vert, .fragment = m_frag}, pipeline_state);
	if (!pipeline) { return; }

	update_and_bind_sets(command_buffer, instances);

	auto& vbo = allocate_scratch(detail::BufferType::eVertexIndex);
	vbo.write(primitive.bytes.data(), primitive.bytes.size());

	command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	command_buffer.setViewport(0, m_viewport);
	command_buffer.setScissor(0, get_scissor(m_renderer->get_render_device().render_view.n_scissor));
	command_buffer.setLineWidth(m_renderer->get_render_device().get_line_width_limits().clamp(line_width));

	auto const instance_count = static_cast<std::uint32_t>(instances.size());
	command_buffer.bindVertexBuffers(0, vbo.get_buffer(), vk::DeviceSize{});
	if (primitive.ibo_offset > 0) {
		command_buffer.bindIndexBuffer(vbo.get_buffer(), primitive.ibo_offset, vk::IndexType::eUint32);
		command_buffer.drawIndexed(primitive.indices, instance_count, 0, 0, 0);
	} else {
		command_buffer.draw(primitive.vertices, instance_count, 0, 0);
	}

	m_sets = {}; // clear for next draw
}

auto Shader::allocate_scratch(detail::BufferType const type) const -> detail::RenderBuffer& {
	return m_renderer->get_render_device().get_buffer_cache().allocate(type);
}

void Shader::set_viewport() {
	auto const fb_extent = m_renderer->get_backbuffer_extent();
	glm::vec2 const viewport = glm::uvec2{fb_extent.width, fb_extent.height};
	m_viewport = vk::Viewport{0.0f, viewport.y, viewport.x, -viewport.y};
}

auto Shader::get_scissor(Rect<> n_rect) const -> vk::Rect2D {
	n_rect.lt = glm::clamp(n_rect.lt, glm::vec2{}, glm::vec2{1.0f});
	n_rect.rb = glm::clamp(n_rect.rb, n_rect.lt, glm::vec2{1.0f});
	auto const fb_extent = m_renderer->get_backbuffer_extent();
	auto const fb_size = glm::vec2{fb_extent.width, fb_extent.height};
	glm::ivec2 const offset = n_rect.lt * fb_size;
	glm::uvec2 const extent = (n_rect.rb - n_rect.lt) * fb_size;
	return vk::Rect2D{vk::Offset2D{offset.x, offset.y}, vk::Extent2D{extent.x, extent.y}};
}

void Shader::update_and_bind_sets(vk::CommandBuffer command_buffer, std::span<RenderInstance::Baked const> instances) const {
	static_assert(detail::set_layout_v.view_instances.set == 0);
	static_assert(detail::set_layout_v.textures.set == 1);
	static_assert(detail::set_layout_v.buffers.set == 2);
	static_assert(detail::set_layout_v.view_instances.bindings[0] == vk::DescriptorType::eUniformBuffer);
	static_assert(detail::set_layout_v.view_instances.bindings[1] == vk::DescriptorType::eStorageBuffer);
	static_assert(detail::set_layout_v.buffers.bindings[0] == vk::DescriptorType::eUniformBuffer);
	static_assert(detail::set_layout_v.buffers.bindings[1] == vk::DescriptorType::eStorageBuffer);

	auto descriptor_sets = allocate_descriptor_sets(m_renderer->get_pipeline_cache());
	static_assert(descriptor_sets.size() == 3);

	auto const vpi_bindings = make_vpi_bindings(m_renderer->get_render_device(), instances);
	auto vpi_write = BufferWrite<vpi_bindings.size()>{};
	make_buffer_write(vpi_write, vpi_bindings, descriptor_sets[0]);

	auto const texture_bindings = make_texture_bindings(m_sets.cis, m_renderer->get_white_texture().combined_image_sampler());
	auto texture_write = ImageWrite<texture_bindings.size()>{};
	make_image_write(texture_write, texture_bindings, descriptor_sets[1]);

	auto const buffer_bindings = make_buffer_bindings(m_renderer->get_render_device().get_buffer_cache(), m_sets.ubo, m_sets.ssbo);
	auto buffer_write = BufferWrite<buffer_bindings.size()>{};
	make_buffer_write(buffer_write, buffer_bindings, descriptor_sets[2]);

	constexpr auto total_writes = vpi_write.writes.size() + texture_write.writes.size() + buffer_write.writes.size();
	auto descriptor_writes = std::array<vk::WriteDescriptorSet, total_writes>{};
	std::size_t index{};
	for (auto const& write : vpi_write.writes) { descriptor_writes[index++] = write; }	   // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
	for (auto const& write : texture_write.writes) { descriptor_writes[index++] = write; } // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
	for (auto const& write : buffer_write.writes) { descriptor_writes[index++] = write; }  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

	m_renderer->get_render_device().get_device().updateDescriptorSets(descriptor_writes, {});

	auto const pipeline_layout = m_renderer->get_pipeline_cache().get_pipeline_layout();
	command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, descriptor_sets, {});
}
} // namespace bave
