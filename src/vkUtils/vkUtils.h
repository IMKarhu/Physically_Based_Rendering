#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>

namespace vkUtils
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

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	QueueFamilyIndices getQueueFamilies(const VkPhysicalDevice pdevice, VkSurfaceKHR surface);

	SwapChainSupportDetails getQuerySwapChainSupportDetails(const VkPhysicalDevice device, const VkSurfaceKHR surface);
}