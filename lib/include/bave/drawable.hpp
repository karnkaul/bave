#pragma once
#include <bave/app.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/mesh.hpp>
#include <bave/graphics/render_instance.hpp>
#include <bave/graphics/set_layout.hpp>
#include <string>
#include <vector>

namespace bave {
class Drawable : public Polymorphic {
  public:
	explicit Drawable(NotNull<App*> app);

	void draw(vk::CommandBuffer command_buffer) const;

	std::string vertex_shader{"shaders/default.vert"};
	std::string fragment_shader{"shaders/default.vert"};

	Transform transform{};
	Rgba tint{};
	std::array<Ptr<Texture const>, SetLayout::max_textures_v> textures{};

	std::vector<RenderInstance> instances{};

  protected:
	void set_geometry(Geometry const& geometry) { m_mesh.write(geometry); }

  private:
	void bake_instances() const;
	void update_textures(Shader& out_shader) const;

	NotNull<App*> m_app;
	mutable std::vector<RenderInstance::Baked> m_baked_instances{};
	Mesh m_mesh;
};
} // namespace bave
