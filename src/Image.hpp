#pragma once

#include "vulkan/vulkan.h"

namespace karhu
{
    // class Device;
    class Image
    {
        public:
            Image();
            Image(VkDevice device,
                    VkPhysicalDevice physicalDevice,
                    uint32_t width,
                    uint32_t height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usageFlags,
                    VkImageAspectFlags aspectFlags,
                    VkMemoryPropertyFlags properties);

            ~Image();

            /*Image(const Image&) = delete;*/
            /*void operator=(const Image&) = delete;*/
            Image(Image&&) noexcept;
            Image& operator=(Image&&) noexcept;

            void createImageView(VkImage image,
                    VkFormat format,
                    VkImageAspectFlags flags,
                    uint32_t layerCount);

            const VkImage& getImage() const { return m_image; }
            const VkImageView& getImageView() const { return m_imageView; }
        private:
            VkImage m_image = VK_NULL_HANDLE;
            VkImageView m_imageView = VK_NULL_HANDLE;
            VkDeviceMemory m_imageMemory;

            VkDevice m_device;
    };
} // karhu namespace
