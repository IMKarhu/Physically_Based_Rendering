#pragma once
#include <vulkan/vulkan.h>
#include <optional>

namespace karhu
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value();
		}
	};

	struct Vulkan_Device
	{
		Vulkan_Device() = default;
		Vulkan_Device(const VkInstance& instance);
		~Vulkan_Device();

		void pickPhysicalDevice();
		void createLogicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue;

		VkInstance m_Instance = VK_NULL_HANDLE;
	};
}