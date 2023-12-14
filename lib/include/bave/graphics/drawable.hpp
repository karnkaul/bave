#pragma once
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/shader.hpp>
#include <bave/graphics/texture.hpp>
#include <memory>
#include <vector>

namespace bave {
class Drawable : public Polymorphic {
  public:
	explicit Drawable(NotNull<RenderDevice*> render_device);

	void draw(Shader& shader, vk::CommandBuffer command_buffer) const;

	Transform transform{};
	Rgba tint{};
	std::array<std::shared_ptr<Texture const>, SetLayout::max_textures_v> textures{};

	std::vector<RenderInstance> instances{};

  protected:
	void set_geometry(Geometry const& geometry) { m_mesh.write(geometry); }
	void set_texture(std::shared_ptr<Texture const> texture) { textures.front() = std::move(texture); }

  private:
	void bake_instances() const;
	void update_textures(Shader& out_shader) const;

	Mesh m_mesh;
	mutable std::vector<RenderInstance::Baked> m_baked_instances{};
};
} // namespace bave
