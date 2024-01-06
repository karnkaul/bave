#pragma once
#include <imgui.h>
#include <bave/core/scoped_resource.hpp>
#include <bave/graphics/texture.hpp>

namespace bave {
class ImTexture {
  public:
	explicit ImTexture(NotNull<std::shared_ptr<Texture>> const& texture);

	[[nodiscard]] auto get_id() const -> ImTextureID { return m_descriptor_set.get(); }
	[[nodiscard]] auto get_texture() const -> std::shared_ptr<Texture> const& { return m_texture.get(); }

  private:
	struct Deleter {
		vk::Device device{};
		void operator()(vk::DescriptorSet descriptor_set) const noexcept;
	};

	NotNull<std::shared_ptr<Texture>> m_texture;
	ScopedResource<vk::DescriptorSet, Deleter> m_descriptor_set;
};
} // namespace bave
