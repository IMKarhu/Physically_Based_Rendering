#pragma once
#include "utils/vkUtils.hpp"
#include <optional>
#ifdef __linux__
#include <experimental/optional>
#endif // DEBUG


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
		Vulkan_Device() = default;
		Vulkan_Device(const VkInstance& instance, const VkSurfaceKHR& surface);
		~Vulkan_Device();

		void pickPhysicalDevice();
		void createLogicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface;
	};
}