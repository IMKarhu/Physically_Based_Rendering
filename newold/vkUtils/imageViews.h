#pragma once
#include "vulkan/vulkan.h"
#include <iostream>

namespace kge
{
	namespace vkUtils
	{
		/*template<typename T>
        void createImageViews(T& imageview, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const VkDevice& device)
		{
			imageview = createImageView(image, format, aspectFlags, device);
		}*/
		static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const VkDevice& device)
		{
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = image;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = aspectFlags;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
            return imageView;
		}
	}
}