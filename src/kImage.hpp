#pragma once
#include <vulkan/vulkan.h>

namespace karhu
{
	struct Vulkan_Device;
	struct kImage
	{
		Vulkan_Device& m_Device;

		void createImage(uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);
	};
}