#include "Texture.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "Image.hpp"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace karhu
{
    Texture::Texture(Device& device)
        : m_device(device) {}

    Texture::~Texture() = default;

    /*Texture::Texture(Texture&& other) noexcept*/
    /*    : m_image(std::move(other.m_image)*/
    /*    , m_device(other.m_device) {}*/

    const VkImage& Texture::getImage() const
    {
        return m_image->getImage();
    }

    const VkImageView& Texture::getImageView() const
    {
        return m_image->getImageView();
    }

    NTexture::NTexture(Device& device, CommandBuffer& commandBuffer, std::string filePath, VkFormat format)
        : Texture(device)
        , m_device(device)
        , m_commandBuffer(commandBuffer)
    {
        std::string f1 = "../textures/";
        std::string f2 = filePath;
        std::string full = f1+f2;
        stbi_uc* pixels = stbi_load(full.c_str(), &m_width, &m_height, &m_nrChannels, STBI_rgb_alpha);
        m_imageSize = m_width * m_height * 4;

        if (!pixels)
        {
            throw std::runtime_error("Unable to load texture!\n");
        }

        VkBuffer staging;
        VkDeviceMemory stagingMemory;
        utils::createBuffers(m_device.lDevice(),
                m_device.pDevice(),
                m_imageSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                staging,
                stagingMemory);
        // m_device.createBuffers(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingMemory);
    
        void* data;
        vkMapMemory(m_device.lDevice(), stagingMemory, 0, m_imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(m_imageSize));
        vkUnmapMemory(m_device.lDevice(), stagingMemory);
    
        stbi_image_free(pixels);

        m_image = std::make_unique<Image>(m_device.lDevice(),
                m_device.pDevice(),
                m_width,
                m_height,
                format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        transitionImageLayout(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(staging);
        
        transitionImageLayout(format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        vkDestroyBuffer(m_device.lDevice(), staging, nullptr);
        vkFreeMemory(m_device.lDevice(), stagingMemory, nullptr);
        
        /*m_image.createImageView(m_image.getImage(), format, VK_IMAGE_ASPECT_COLOR_BIT, 1);*/
        createSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    }

    NTexture::~NTexture() {}

    NTexture::NTexture(NTexture&& other) noexcept
        : Texture(std::move(other))
        , m_width(other.m_width)
        , m_height(other.m_height)
        , m_nrChannels(other.m_nrChannels)
        , m_imageSize(other.m_imageSize)
        , m_sampler(other.m_sampler)
        , m_device(other.m_device)
        , m_commandBuffer(other.m_commandBuffer) {}

    NTexture& NTexture::operator=(NTexture&& other) noexcept
    {
        if (this != &other)
        {
            Texture::operator=(std::move(other));
            m_width = other.m_width;
            m_height = other.m_height;
            m_nrChannels = other.m_nrChannels;
            m_imageSize = other.m_imageSize;
            m_sampler = other.m_sampler;
        }
        return *this;
    }

    void NTexture::transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = m_commandBuffer.recordSingleCommand();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image->getImage();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage , destinationStage ,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        m_commandBuffer.endSingleCommand(commandBuffer);
    }

    void NTexture::copyBufferToImage(VkBuffer buffer)
    {
        VkCommandBuffer commandBuffer = m_commandBuffer.recordSingleCommand();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            static_cast<uint32_t>(m_width),
            static_cast<uint32_t>(m_height),
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            m_image->getImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        m_commandBuffer.endSingleCommand(commandBuffer);

    }
    void NTexture::createSampler(VkSamplerAddressMode addressMode)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_device.pDevice(), &properties);

        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_device.lDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
    VkDescriptorImageInfo NTexture::getImageInfo()
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_image->getImageView();
        imageInfo.sampler = m_sampler;
        return imageInfo;
    }


    // void Texture::createTexture(std::string filepath, VkFormat format)
    // {
    //     std::string path1 = "../textures/";
    //     std::string path = path1 + filepath;
    //     int width, height, nrChannels;
    //     stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
    //     VkDeviceSize imageSize = width * height * 4;
    //
    //     if (!pixels)
    //     {
    //     	throw std::runtime_error("unable to load texture!\n");
    //     }
    //
    //     VkBuffer staging;
    //     VkDeviceMemory stagingMemory;
    //     m_device.createBuffers(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingMemory);
    //
    //     void* data;
    //     vkMapMemory(m_device.lDevice(), stagingMemory, 0, imageSize, 0, &data);
    //     memcpy(data, pixels, static_cast<size_t>(imageSize));
    //     vkUnmapMemory(m_device.lDevice(), stagingMemory);
    //
    //     stbi_image_free(pixels);
    //
    //     Image image{ .m_Device = m_device };
    //     kImage.createImage(width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureVars.m_texture, m_TextureVars.m_Memory, 1, 0);
    //
    //     transitionImagelayout(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //
    //     copyBufferToImage(staging, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    //
    //     transitionImagelayout(format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //
    //     vkDestroyBuffer(m_device.lDevice(), staging, nullptr);
    //     vkFreeMemory(m_device.lDevice(), stagingMemory, nullptr);
    //
    //     textureImageView(format, 1, false);
    //     createSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    // }
    //
    // void Texture::createHDRTexture(std::string filepath, VkFormat format)
    // {
    //     stbi_set_flip_vertically_on_load(true);
    //     std::string path1 = "../textures/";
    //     std::string path = path1 + filepath;
    //     int width, height, nrComponents;
    //     float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
    //     VkDeviceSize imageSize = width * height * sizeof(float) * 3; //RGB float32
    //     unsigned int hdrTexture;
    //
    //     if (!data)
    //     {
    //         std::cout << "failed to load HDR image" << std::endl;
    //     }
    //
    //     VkBuffer staging;
    //     VkDeviceMemory stagingMemory;
    //     m_Device.createBuffers(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingMemory);
    //
    //     void* voidData;
    //     vkMapMemory(m_device.lDevice(), stagingMemory, 0, imageSize, 0, &voidData);
    //     memcpy(voidData, data, static_cast<size_t>(imageSize));
    //     vkUnmapMemory(m_device.lDevice(), stagingMemory);
    //
    //     stbi_image_free(data);
    //
    //     kImage kImage{ .m_Device = m_Device };
    //     kImage.createImage(width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureVars.m_texture, m_TextureVars.m_Memory, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
    //
    //     transitionImagelayout(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //
    //     copyBufferToImageCube(staging, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    //
    //     transitionImagelayout(format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //
    //     vkDestroyBuffer(m_device.lDevice(), staging, nullptr);
    //     vkFreeMemory(m_device.lDevice(), stagingMemory, nullptr);
    //
    //
    //     textureImageView(format, 6, false);
    //     createSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    // }
    //
    // void Texture::transitionImagelayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    // {
    //     VkCommandBuffer commandBuffer = m_SwapChain.RecordSingleCommand();
    //
    //     VkImageMemoryBarrier barrier{};
    //     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //     barrier.oldLayout = oldLayout;
    //     barrier.newLayout = newLayout;
    //     barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //     barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //     barrier.image = m_TextureVars.m_texture;
    //     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //     barrier.subresourceRange.baseMipLevel = 0;
    //     barrier.subresourceRange.levelCount = 1;
    //     barrier.subresourceRange.baseArrayLayer = 0;
    //     barrier.subresourceRange.layerCount = 1;
    //     VkPipelineStageFlags sourceStage;
    //     VkPipelineStageFlags destinationStage;
    //
    //     if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    //         barrier.srcAccessMask = 0;
    //         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //
    //         sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //         destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //     }
    //     else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    //         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //
    //         sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //         destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //     }
    //     else {
    //         throw std::invalid_argument("unsupported layout transition!");
    //     }
    //
    //     vkCmdPipelineBarrier(
    //         commandBuffer,
    //         sourceStage , destinationStage ,
    //         0,
    //         0, nullptr,
    //         0, nullptr,
    //         1, &barrier);
    //
    //     m_SwapChain.endSingleCommand(commandBuffer);
    // }
    // void Texture::copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height)
    // {
    //     VkCommandBuffer commandBuffer = m_SwapChain.RecordSingleCommand();
    //
    //     VkBufferImageCopy region{};
    //     region.bufferOffset = 0;
    //     region.bufferRowLength = 0;
    //     region.bufferImageHeight = 0;
    //
    //     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //     region.imageSubresource.mipLevel = 0;
    //     region.imageSubresource.baseArrayLayer = 0;
    //     region.imageSubresource.layerCount = 1;
    //
    //     region.imageOffset = { 0, 0, 0 };
    //     region.imageExtent = {
    //         width,
    //         height,
    //         1
    //     };
    //
    //     vkCmdCopyBufferToImage(
    //         commandBuffer,
    //         buffer,
    //         m_TextureVars.m_texture,
    //         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //         1,
    //         &region
    //     );
    //
    //     m_SwapChain.endSingleCommand(commandBuffer);
    // }
    //
    // void Texture::copyBufferToImageCube(VkBuffer buffer, uint32_t width, uint32_t height)
    // {
    //     VkCommandBuffer commandBuffer = m_SwapChain.RecordSingleCommand();
    //     std::vector<VkBufferImageCopy> copyRegions;
    //
    //     for (size_t i = 0; i < 6; i++)
    //     {
    //         VkBufferImageCopy copyRegion{};
    //         copyRegion.bufferOffset = 0;
    //         copyRegion.bufferRowLength = 0;
    //         copyRegion.bufferImageHeight = 0;
    //
    //         copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //         copyRegion.imageSubresource.mipLevel = 0;
    //         copyRegion.imageSubresource.baseArrayLayer = i;
    //         copyRegion.imageSubresource.layerCount = 1;
    //
    //         copyRegion.imageOffset = { 0, 0, 0 };
    //         copyRegion.imageExtent = {
    //             width,
    //             height,
    //             1
    //         };
    //         copyRegions.push_back(copyRegion);
    //     }
    //
    //     vkCmdCopyBufferToImage(
    //         commandBuffer,
    //         buffer,
    //         m_TextureVars.m_texture,
    //         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //         static_cast<uint32_t>(copyRegions.size()),
    //         copyRegions.data()
    //     );
    //
    //     m_SwapChain.endSingleCommand(commandBuffer);
    // }
    //
    // void Texture::textureImageView(VkFormat format, uint32_t layerCount, bool cubemap)
    // {
    //     if (cubemap)
    //     {
    //         VkImageView view = m_SwapChain.createImageView(m_TextureVars.m_texture, format, VK_IMAGE_ASPECT_COLOR_BIT, layerCount);
    //         m_TextureVars.m_ImageViews.push_back(view);
    //         return;
    //     }
    //     m_TextureVars.m_TextureView = m_SwapChain.createImageView(m_TextureVars.m_texture, format, VK_IMAGE_ASPECT_COLOR_BIT, layerCount);
    // }
    //
    // void Texture::createSampler(VkSamplerAddressMode addressMode)
    // {
    //     VkSamplerCreateInfo samplerInfo{};
    //     samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //     samplerInfo.magFilter = VK_FILTER_LINEAR;
    //     samplerInfo.minFilter = VK_FILTER_LINEAR;
    //     samplerInfo.addressModeU = addressMode;
    //     samplerInfo.addressModeV = addressMode;
    //     samplerInfo.addressModeW = addressMode;
    //     samplerInfo.anisotropyEnable = VK_TRUE;
    //
    //     VkPhysicalDeviceProperties properties{};
    //     vkGetPhysicalDeviceProperties(m_Device.m_PhysicalDevice, &properties);
    //
    //     samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    //     samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    //     samplerInfo.unnormalizedCoordinates = VK_FALSE;
    //     samplerInfo.compareEnable = VK_FALSE;
    //     samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    //     samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    //     samplerInfo.mipLodBias = 0.0f;
    //     samplerInfo.minLod = 0.0f;
    //     samplerInfo.maxLod = 0.0f;
    //
    //     if (vkCreateSampler(m_device.lDevice(), &samplerInfo, nullptr, &m_TextureVars.m_Sampler) != VK_SUCCESS) {
    //         throw std::runtime_error("failed to create texture sampler!");
    //     }
    // }
    //
    // VkDescriptorImageInfo Texture::getImageInfo()
    // {
    //     VkDescriptorImageInfo imageInfo{};
    //     imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //     imageInfo.imageView = m_TextureVars.m_TextureView;
    //     imageInfo.sampler = m_TextureVars.m_Sampler;
    //     return imageInfo;
    // }
}
