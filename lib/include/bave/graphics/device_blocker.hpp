#pragma once
#include <bave/core/scoped_resource.hpp>
#include <vulkan/vulkan.hpp>

namespace bave {
struct DeviceWaiter {
	void operator()(vk::Device device) const { device.waitIdle(); }
};

using DeviceBlocker = ScopedResource<vk::Device, DeviceWaiter>;
} // namespace bave
