#pragma once

#include "vulkan/vulkan.h"

namespace karhu
{
    class Device;
    class Image
    {
        public:
            Image(Device& device,
                    uint32_t width,
                    uint32_t height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usageFlags,
                    VkMemoryPropertyFlags properties);

            ~Image();

            Image(const Image&) = delete;
            void operator=(const Image&) = delete;
            Image(Image&&) = delete;
            Image& operator=(Image&&) = delete;

            void createImageView(VkImage image,
                    VkFormat format,
                    VkImageAspectFlags flags,
                    uint32_t layerCount);

            const VkImageView& getImageView() const { return m_imageView; }
        private:
            VkImage m_image;
            VkImageView m_imageView;
            VkDeviceMemory m_imageMemory;

            Device& m_device;
    };
} // karhu namespace
