#include "Image.hpp"

#include "Device.hpp"

namespace karhu
{
    Image::Image(Device& device,
                    uint32_t width,
                    uint32_t height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usageFlags,
                    VkMemoryPropertyFlags properties)
        : m_device(device)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usageFlags;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateImage(m_device.lDevice(), &imageInfo, nullptr, &m_image));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device.lDevice(), m_image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_device.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device.lDevice(), &allocInfo, nullptr, &m_imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_device.lDevice(), m_image, m_imageMemory, 0);

        createImageView(m_image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

    Image::~Image()
    {
        printf("calling Image class destructor! \n");
    }

    void Image::createImageView(VkImage image,
            VkFormat format,
            VkImageAspectFlags flags,
            uint32_t layerCount)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.subresourceRange.aspectMask = flags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = layerCount;

        VK_CHECK(vkCreateImageView(m_device.lDevice(), &createInfo, nullptr, &m_imageView));
    }

} // karhu namespace
