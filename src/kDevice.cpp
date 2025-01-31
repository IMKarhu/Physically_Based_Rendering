#include "kDevice.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace karhu
{
	Vulkan_Device::Vulkan_Device(const VkInstance& instance, const VkSurfaceKHR& surface)
		:m_Instance(instance)
		,m_Surface(surface)
	{
		init();
	}

	Vulkan_Device::~Vulkan_Device()
	{
		printf("device destroyed\n");
		if (enableValidationLayers)
		{
			destroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}
		vkDestroyDevice(m_Device, nullptr);
	}

	void Vulkan_Device::init()
	{
		setupDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
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

	void Vulkan_Device::createBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createinfo.size = size; //byte size of one vertices multiplied by size of vector
		createinfo.usage = usage;
		createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateBuffer(m_Device, &createinfo, nullptr, &buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocinfo.allocationSize = memRequirements.size;
		/*VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/
		allocinfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		VK_CHECK(vkAllocateMemory(m_Device, &allocinfo, nullptr, &bufferMemory));
		//  printf("Hello\n");
		VK_CHECK(vkBindBufferMemory(m_Device, buffer, bufferMemory, 0));
	}

	void Vulkan_Device::copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_CHECK(vkQueueWaitIdle(m_GraphicsQueue));
		vkFreeCommandBuffers(m_Device, commandPool, 1, &commandBuffer);
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
	VkFormat Vulkan_Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
			else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("Could not find Supported format!\n");
	}
	void Vulkan_Device::setupDebugMessenger()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createinfo{};
		vkUtils::populateDebugMessengerCreateInfo(createinfo);

		if (createDebugUtilsMessengerEXT(m_Instance, &createinfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create debug messenger!\n");
		}
	}
	VkResult Vulkan_Device::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void Vulkan_Device::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}