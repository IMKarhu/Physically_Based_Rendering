#pragma once
#include "utils/vkUtils.hpp"
#include <optional>
//#include <experimental/optional>
#include <set>

namespace karhu
{
	struct QueueFamilyIndices
	{

		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};


	struct Vulkan_Device
	{
		//Vulkan_Device() = default;
		Vulkan_Device(const VkInstance& instance, const VkSurfaceKHR& surface);
		~Vulkan_Device();

		Vulkan_Device(const Vulkan_Device&) = delete;
		void operator=(const Vulkan_Device&) = delete;
		Vulkan_Device(Vulkan_Device&&) = delete;
		Vulkan_Device& operator=(Vulkan_Device&&) = delete;

		void init();
		void pickPhysicalDevice();
		void createLogicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		void createBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandPool);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	};
}