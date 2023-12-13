#pragma once
#include <bave/graphics/gpu.hpp>
#include <span>

namespace bave::detail {
class InstanceBuilder {
  public:
	struct Result {
		vk::UniqueInstance instance{};
		vk::UniqueDebugUtilsMessengerEXT debug_messenger{};

		[[nodiscard]] auto is_validation_enabled() const -> bool { return !!debug_messenger; }
	};

	std::span<char const* const> extensions{};
	bool validation{};

	[[nodiscard]] auto build() -> Result;
};

class DeviceBuilder {
  public:
	struct Result {
		vk::UniqueDevice device{};
		vk::Queue queue{};
	};

	explicit DeviceBuilder(vk::Instance instance, vk::SurfaceKHR surface);

	Gpu gpu{};

	[[nodiscard]] auto get_gpus() const -> std::span<Gpu const> { return m_gpus; }

	[[nodiscard]] auto build() const -> Result;

  private:
	vk::Instance m_instance{};
	vk::SurfaceKHR m_surface{};
	std::vector<Gpu> m_gpus{};
};
} // namespace bave::detail
