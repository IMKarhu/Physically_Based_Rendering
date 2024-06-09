#include "kDevice.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace karhu
{
	Vulkan_Device::Vulkan_Device(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		m_Instance = instance;
		m_Surface = surface;
	}

	Vulkan_Device::~Vulkan_Device()
	{
		printf("device destroyed\n");
		vkDestroyDevice(m_Device, nullptr);
	}

	void Vulkan_Device::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		VK_CHECK(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr));

		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find GPU with a vulkan support!\n");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		VK_CHECK(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data()));

		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}
		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find suitable GPU!\n");
		}

	}

	void Vulkan_Device::createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> quecreateinfos;
		std::set<uint32_t> uniqueQueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		for (uint32_t queFamily : uniqueQueFamilies)
		{
			VkDeviceQueueCreateInfo createinfo{};
			createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createinfo.queueFamilyIndex = queFamily;
			createinfo.queueCount = 1;
			float queuePriority = 1.0f; /* This needs to be between 0.0 - 1.0. */
			createinfo.pQueuePriorities = &queuePriority;
			quecreateinfos.push_back(createinfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{}; /* Leave this as vk_false for now, still need it for device creation. */

		VkDeviceCreateInfo deviceinfo{};
		deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceinfo.queueCreateInfoCount = static_cast<uint32_t>(uniqueQueFamilies.size());
		deviceinfo.pQueueCreateInfos = quecreateinfos.data();
		deviceinfo.pEnabledFeatures = &deviceFeatures;
		deviceinfo.enabledExtensionCount = static_cast<uint32_t>(vkUtils::deviceExtensions.size());
		deviceinfo.ppEnabledExtensionNames = vkUtils::deviceExtensions.data();

		if (enableValidationLayers)
		{
			deviceinfo.enabledLayerCount = static_cast<uint32_t>(vkUtils::valiadationLayers.size());
			deviceinfo.ppEnabledLayerNames = vkUtils::valiadationLayers.data();
		}
		else
		{
			deviceinfo.enabledLayerCount = 0;
		}

		VK_CHECK(vkCreateDevice(m_PhysicalDevice, &deviceinfo, nullptr, &m_Device));

		vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	}

	bool Vulkan_Device::isDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = vkUtils::checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	QueueFamilyIndices Vulkan_Device::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queFamily : queueFamilies)
		{
			if (queFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}

		return indices;
	}

	SwapChainSupportDetails Vulkan_Device::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities));

		uint32_t formaCount;
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formaCount, nullptr));

		if (formaCount != 0)
		{
			details.formats.resize(formaCount);
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formaCount, details.formats.data()));
		}

		uint32_t presentModeCount;
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr));

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data()));
		}

		return details;
	}
	uint32_t Vulkan_Device::findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memoryProperties);
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((filter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
	}
}