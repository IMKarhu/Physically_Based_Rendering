#include "Image.hpp"

#include "Device.hpp"
#include "utils/utils.hpp"

namespace karhu
{
    Image::Image() {}
    Image::Image(VkDevice device,
            VkPhysicalDevice physicalDevice,
            uint32_t mipLevels,
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usageFlags,
            VkImageAspectFlags aspectFlags,
            VkMemoryPropertyFlags properties,
            bool isCubeMap)
        : m_device(device)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        if (isCubeMap)
        {
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            imageInfo.arrayLayers = 6;
        }
        else
        {
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.flags = 0;
            imageInfo.arrayLayers = 1;
        }
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usageFlags;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateImage(m_device, &imageInfo, nullptr, &m_image));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, m_image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = utils::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_device, m_image, m_imageMemory, 0);

        createImageView(m_image, format, aspectFlags, mipLevels, isCubeMap);
        if (isCubeMap)
        {
            for(size_t face = 0; face < 6; ++face)
            {
                createImageViewPerFace(m_image, format, aspectFlags, mipLevels, face);
            }
        }
    }

    Image::Image(Image&& other) noexcept
        : m_image(other.m_image)
        , m_imageView(other.m_imageView)
        , m_faceImageViews(other.m_faceImageViews)
        , m_imageMemory(other.m_imageMemory)
        , m_sampler(other.m_sampler)
        , m_device(other.m_device)
    {
        other.m_image = VK_NULL_HANDLE;
        other.m_imageView = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
        other.m_imageMemory = VK_NULL_HANDLE;
        other.m_sampler = VK_NULL_HANDLE;
        for(size_t i = 0; i < m_faceImageViews.size(); i++)
        {
            m_faceImageViews[i] = VK_NULL_HANDLE;
        }
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            if (m_image != VK_NULL_HANDLE)
            {
                vkDestroyImage(m_device, m_image, nullptr);
                vkDestroyImageView(m_device, m_imageView, nullptr);
                vkFreeMemory(m_device, m_imageMemory, nullptr);
                vkDestroySampler(m_device, m_sampler, nullptr);
                for(size_t i = 0; i < m_faceImageViews.size(); i++)
                {
                    vkDestroyImageView(m_device, m_faceImageViews[i], nullptr);
                }
            }

            m_image = other.m_image;
            m_imageView = other.m_imageView;
            m_device = other.m_device;
            m_imageMemory = other.m_imageMemory;
            m_faceImageViews = other.m_faceImageViews;
            m_sampler = other.m_sampler;

            other.m_image = VK_NULL_HANDLE;
            other.m_imageView = VK_NULL_HANDLE;
            other.m_device = VK_NULL_HANDLE;
            other.m_imageMemory = VK_NULL_HANDLE;
            other.m_sampler = VK_NULL_HANDLE;
            for(size_t i = 0; i < m_faceImageViews.size(); i++)
            {
                m_faceImageViews[i] = VK_NULL_HANDLE;
            }
        }
        return *this;
    }

    Image::~Image()
    {
        /*printf("calling Image class destructor! \n");*/
        /*vkDestroyImage(m_device, m_image, nullptr);*/
        /*vkDestroyImageView(m_device, m_imageView, nullptr);*/
        /*vkFreeMemory(m_device, m_imageMemory, nullptr);*/
    }

    void Image::createImageView(VkImage image,
            VkFormat format,
            VkImageAspectFlags flags,
            uint32_t mipLevels,
            bool isCubeMap)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.format = format;
        createInfo.subresourceRange.aspectMask = flags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.baseArrayLayer = 0;
        if (isCubeMap)
        {
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        createInfo.subresourceRange.layerCount = 6;
        }
        else
        {
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.subresourceRange.layerCount = 1;
        }


        VK_CHECK(vkCreateImageView(m_device, &createInfo, nullptr, &m_imageView));
    }

    void Image::createImageViewPerFace(VkImage image,
            VkFormat format,
            VkImageAspectFlags flags,
            uint32_t mipLevels,
            size_t face)
    {
        m_faceImageViews.resize(6);
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.format = format;
        createInfo.subresourceRange.aspectMask = flags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.baseArrayLayer = face;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(m_device, &createInfo, nullptr, &m_faceImageViews[face]));
    }

    void Image::createSampler(VkDevice device, uint32_t mipLevels)
    {
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.minLod = 0.0f;
        samplerCI.maxLod = static_cast<float>(mipLevels);
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK(vkCreateSampler(device, &samplerCI, nullptr, &m_sampler));
    }

    VkDescriptorImageInfo Image::imageInfo()
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_imageView;
        imageInfo.sampler = m_sampler;
        return imageInfo;
    }

} // karhu namespace
