#pragma once
#include <bave/graphics/drawable.hpp>

namespace bave {
/// \brief Instanced rendering of a Drawable.
template <std::derived_from<Drawable> Type>
class Instanced : public Type {
  public:
	/// \brief Number of instances to draw.
	std::vector<RenderInstance> instances{};
	/// \brief Whether to parent RenderInstance transforms on transform.
	///
	/// If true, instance.transform will be parented to this->transform.
	bool parented_instances{true};

	/// \brief Draw instances using a given shader.
	/// \param shader Shader to use.
	void draw(Shader& shader) const override {
		bake_instances();
		this->update_textures(shader);
		shader.draw(this->get_render_primitive(), m_baked_instances);
	}

  private:
	void bake_instances() const {
		m_baked_instances.clear();
		auto const parent = parented_instances ? this->transform.matrix() : glm::identity<glm::mat4>();
		RenderInstance::fill_baked(m_baked_instances, instances, parent);
	}

	mutable std::vector<RenderInstance::Baked> m_baked_instances{};
};
} // namespace bave
