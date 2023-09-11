#pragma once
#include "vulkan/vulkan.h"
#include <optional>
#include <vector>

namespace kge
{
	namespace vkUtils
	{
		/*struct Queues
		{
			VkQueue m_GraphicsQueue{};
			VkQueue m_PresentQueue{};
		};*/

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		/* List of required device extensions. */
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& pdevice, const VkSurfaceKHR& surface);
	}
}