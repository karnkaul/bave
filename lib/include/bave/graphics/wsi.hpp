#pragma once
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/gpu.hpp>
#include <span>

namespace bave {
class IWsi : public Polymorphic {
  public:
	[[nodiscard]] virtual auto get_instance_extensions() const -> std::span<char const* const> = 0;
	[[nodiscard]] virtual auto make_surface(vk::Instance instance) const -> vk::SurfaceKHR = 0;
	[[nodiscard]] virtual auto get_framebuffer_extent() const -> vk::Extent2D = 0;

	[[nodiscard]] virtual auto select_gpu(std::span<Gpu const> gpus) const -> Gpu { return gpus.front(); }
};
} // namespace bave
