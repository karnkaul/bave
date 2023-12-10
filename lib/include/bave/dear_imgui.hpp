#pragma once
#include <bave/core/ptr.hpp>
#include <bave/core/scoped_resource.hpp>
#include <bave/graphics/device_blocker.hpp>
#include <bave/platform.hpp>

static_assert(bave::imgui_v);

struct GLFWwindow;

namespace bave {
class RenderDevice;

class DearImGui {
  public:
	explicit DearImGui(Ptr<GLFWwindow> window, RenderDevice& render_device, vk::RenderPass render_pass);

	void new_frame();
	void end_frame();
	void render(vk::CommandBuffer command_buffer);

  private:
	struct Instance {
		bool init{};
		auto operator==(Instance const&) const -> bool = default;

		struct Deleter {
			void operator()(Instance instance) const noexcept;
		};
	};

	enum class State { eNewFrame, eEndFrame };

	ScopedResource<Instance, Instance::Deleter> m_instance{};
	vk::UniqueDescriptorPool m_pool{};
	State m_state{};

	DeviceBlocker m_blocker{};
};
} // namespace bave
