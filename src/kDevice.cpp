#include "kDevice.hpp"
#include "utils/vkUtils.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace karhu
{
	Vulkan_Device::Vulkan_Device(const VkInstance& instance)
	{
		m_Instance = instance;
	}

	Vulkan_Device::~Vulkan_Device()
	{
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

		VkDeviceQueueCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createinfo.queueFamilyIndex = indices.graphicsFamily.value();
		createinfo.queueCount = 1;

		float queuePriority = 1.0f; /* This needs to be between 0.0 - 1.0. */
		createinfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{}; /* Leave this as vk_false for now, still need it for device creation. */

		VkDeviceCreateInfo deviceinfo{};
		deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceinfo.pQueueCreateInfos = &createinfo;
		deviceinfo.queueCreateInfoCount = 1;
		deviceinfo.pEnabledFeatures = &deviceFeatures;
		deviceinfo.enabledExtensionCount = 0;

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
	}

	bool Vulkan_Device::isDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		QueueFamilyIndices indices = findQueueFamilies(device);

		return indices.isComplete();
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

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}

		return indices;
	}
}