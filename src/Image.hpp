#pragma once

#include "vulkan/vulkan.h"

#include <vector>

namespace karhu
{
    // class Device;
    class Image
    {
        public:
            Image();
            Image(VkDevice device,
                    VkPhysicalDevice physicalDevice,
                    uint32_t mipLevels,
                    uint32_t width,
                    uint32_t height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usageFlags,
                    VkImageAspectFlags aspectFlags,
                    VkMemoryPropertyFlags properties,
                    bool isCubeMap = false);

            ~Image();

            /*Image(const Image&) = delete;*/
            /*void operator=(const Image&) = delete;*/
            Image(Image&&) noexcept;
            Image& operator=(Image&&) noexcept;

            void createImageView(VkImage image,
                    VkFormat format,
                    VkImageAspectFlags flags,
                    uint32_t mipLevels,
                    bool isCubeMap = false);
            void createImageViewPerFace(VkImage image,
                    VkFormat format,
                    VkImageAspectFlags flags,
                    uint32_t mipLevels,
                    size_t face);
            //for now only for generating BRDF lookup table.
            void createSampler(VkDevice device, uint32_t mipLevels);

            const VkImage& getImage() const { return m_image; }
            const VkImageView& getImageView() const { return m_imageView; }
            const VkSampler& getSampler() const { return m_sampler; }
            const std::vector<VkImageView>& getImageViews() const { return m_faceImageViews; }
        private:
            VkImage m_image = VK_NULL_HANDLE;
            VkImageView m_imageView = VK_NULL_HANDLE;
            VkSampler m_sampler = VK_NULL_HANDLE;
            std::vector<VkImageView> m_faceImageViews;
            VkDeviceMemory m_imageMemory;

            VkDevice m_device;
    };
} // karhu namespace
