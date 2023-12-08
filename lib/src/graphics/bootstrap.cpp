#include <bave/build_version.hpp>
#include <bave/core/error.hpp>
#include <bave/logger.hpp>
#include <graphics/bootstrap.hpp>
#include <algorithm>

namespace bave {
namespace {
[[nodiscard]] auto to_vk_version(Version const& v) -> std::uint32_t { return VK_MAKE_VERSION(v.major, v.minor, v.patch); }

auto const g_log = Logger{"Vulkan"};

auto make_instance(std::vector<char const*> extensions, bool& out_validation) -> vk::UniqueInstance {
	static constexpr std::string_view validation_layer_v = "VK_LAYER_KHRONOS_validation";
	auto vdl = vk::DynamicLoader{};
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vdl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

	auto const api_version = vk::enumerateInstanceVersion();
	g_log.info("Vulkan API version: v{}.{}.{}", VK_VERSION_MAJOR(api_version), VK_VERSION_MINOR(api_version), VK_VERSION_PATCH(api_version));

	if (out_validation) {
		auto const available_layers = vk::enumerateInstanceLayerProperties();
		auto layer_search = [](vk::LayerProperties const& properties) { return properties.layerName.data() == validation_layer_v; };
		if (std::find_if(available_layers.begin(), available_layers.end(), layer_search) != available_layers.end()) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		} else {
			g_log.warn("Validation layers requested but not available");
			out_validation = false;
		}
	}

	auto const version = to_vk_version(build_version_v);
	auto const vai = vk::ApplicationInfo{"bave", version, "bave", version, VK_API_VERSION_1_2};
	auto ici = vk::InstanceCreateInfo{};
	ici.pApplicationInfo = &vai;
#if defined(__APPLE__)
	ici.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif
	ici.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
	ici.ppEnabledExtensionNames = extensions.data();
	if (out_validation) {
		static constexpr char const* layers = validation_layer_v.data();
		ici.enabledLayerCount = 1;
		ici.ppEnabledLayerNames = &layers;
	}
	auto ret = vk::UniqueInstance{};
	try {
		ret = vk::createInstanceUnique(ici);
	} catch (vk::LayerNotPresentError const& e) {
		g_log.error("{}", e.what());
		ici.enabledLayerCount = 0;
		ret = vk::createInstanceUnique(ici);
	}
	if (!ret) { throw Error{"Failed to createe Vulkan Instance"}; }
	VULKAN_HPP_DEFAULT_DISPATCHER.init(ret.get());
	return ret;
}

auto make_debug_messenger(vk::Instance instance) -> vk::UniqueDebugUtilsMessengerEXT {
	auto validationCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT,
								 VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void*) -> vk::Bool32 {
		std::string_view const msg = (pCallbackData != nullptr) && (pCallbackData->pMessage != nullptr) ? pCallbackData->pMessage : "UNKNOWN";
		switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: g_log.error("{}", msg); return vk::True;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: g_log.warn("{}", msg); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: g_log.info("{}", msg); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: g_log.debug("{}", msg); break;
		default: break;
		}
		return vk::False;
	};

	auto dumci = vk::DebugUtilsMessengerCreateInfoEXT{};
	using vksev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	dumci.messageSeverity = vksev::eError | vksev::eWarning | vksev::eInfo | vksev::eVerbose;
	using vktype = vk::DebugUtilsMessageTypeFlagBitsEXT;
	dumci.messageType = vktype::eGeneral | vktype::ePerformance | vktype::eValidation;
	dumci.pfnUserCallback = validationCallback;
	try {
		return instance.createDebugUtilsMessengerEXTUnique(dumci, nullptr);
	} catch (std::exception const& e) { throw Error{"{}", e.what()}; }
}

auto get_ranked_gpus(vk::Instance const instance, vk::SurfaceKHR const surface) -> std::vector<Gpu> {
	enum class Preference {
		eDiscrete = 10,
	};
	struct Entry {
		Gpu gpu{};
		int preference{};
		auto operator<=>(Entry const& rhs) const { return preference <=> rhs.preference; }
	};
	auto const get_queue_family = [surface](vk::PhysicalDevice const& device, std::uint32_t& out_family) {
		static constexpr auto queue_flags_v = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer;
		auto const properties = device.getQueueFamilyProperties();
		for (size_t i = 0; i < properties.size(); ++i) {
			auto const family = static_cast<std::uint32_t>(i);
			if (device.getSurfaceSupportKHR(family, surface) == 0) { continue; }
			if (!(properties[i].queueFlags & queue_flags_v)) { continue; }
			out_family = family;
			return true;
		}
		return false;
	};
	auto const devices = instance.enumeratePhysicalDevices();
	auto entries = std::vector<Entry>{};
	for (auto const& device : devices) {
		auto entry = Entry{.gpu = {device}};
		entry.gpu.properties = device.getProperties();
		if (!get_queue_family(device, entry.gpu.queue_family)) { continue; }
		if (entry.gpu.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) { entry.preference += static_cast<int>(Preference::eDiscrete); }
		entries.push_back(entry);
	}
	if (entries.empty()) { throw Error{"Failed to find a suitable Vulkan Physical Device"}; }
	std::sort(entries.begin(), entries.end());

	auto ret = std::vector<Gpu>{};
	for (auto const& entry : entries) { ret.push_back(entry.gpu); }
	return ret;
}

auto make_device(Gpu const& gpu) -> vk::UniqueDevice {
	static constexpr float priority_v = 1.0f;
	static constexpr std::array required_extensions_v = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,

#if defined(__APPLE__)
		"VK_KHR_portability_subset",
#endif
	};

	auto qci = vk::DeviceQueueCreateInfo{{}, gpu.queue_family, 1, &priority_v};
	auto dci = vk::DeviceCreateInfo{};
	auto enabled = vk::PhysicalDeviceFeatures{};
	auto available_features = gpu.device.getFeatures();
	enabled.fillModeNonSolid = available_features.fillModeNonSolid;
	enabled.wideLines = available_features.wideLines;
	enabled.samplerAnisotropy = available_features.samplerAnisotropy;
	enabled.sampleRateShading = available_features.sampleRateShading;
	auto const available_extensions = gpu.device.enumerateDeviceExtensionProperties();
	for (auto const* ext : required_extensions_v) {
		auto const found = [ext](vk::ExtensionProperties const& props) { return std::string_view{props.extensionName} == ext; };
		if (std::ranges::find_if(available_extensions, found) == available_extensions.end()) {
			throw Error{"Required extension '{}' not supported by selected GPU '{}'", ext, gpu.properties.deviceName.data()};
		}
	}

	dci.queueCreateInfoCount = 1;
	dci.pQueueCreateInfos = &qci;
	dci.enabledExtensionCount = static_cast<std::uint32_t>(required_extensions_v.size());
	dci.ppEnabledExtensionNames = required_extensions_v.data();
	dci.pEnabledFeatures = &enabled;

	auto ret = gpu.device.createDeviceUnique(dci);
	if (!ret) { throw Error{"Failed to create Vulkan Device"}; }

	VULKAN_HPP_DEFAULT_DISPATCHER.init(ret.get());
	return ret;
}
} // namespace

auto InstanceBuilder::build() -> Result {
	auto ret = Result{.instance = make_instance({extensions.begin(), extensions.end()}, validation)};
	if (validation) { ret.debug_messenger = make_debug_messenger(*ret.instance); }
	return ret;
}

DeviceBuilder::DeviceBuilder(vk::Instance instance, vk::SurfaceKHR surface) : m_instance(instance), m_surface(surface) {
	if (!m_instance || !m_surface) { throw Error{"Uninitialized Vulkan Instance/Surface"}; }
	m_gpus = get_ranked_gpus(m_instance, m_surface);
	if (m_gpus.empty()) { throw Error{"No suitable Vulkan GPU found"}; }
	gpu = m_gpus.front();
}

auto DeviceBuilder::build() const -> Result {
	if (!gpu.device) { throw Error{"Uninitialized GPU"}; }
	auto ret = Result{.device = make_device(gpu)};
	ret.queue = ret.device->getQueue(gpu.queue_family, 0);
	return ret;
}
} // namespace bave
