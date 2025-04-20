#include "kTexture.hpp"
#include "kDevice.hpp"
#include "kSwapChain.hpp"
#include "kImage.hpp"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace karhu
{
	kTexture::kTexture(Vulkan_Device& device, Vulkan_SwapChain& swapchain, std::string filepath, VkFormat format)
		: m_Device(device)
        , m_SwapChain(swapchain)
	{
        createTexture(filepath, format);
	}

    kTexture::~kTexture()
    {
        //printf("texture destructor called!!");
       /* vkDestroySampler(m_Device.m_Device, m_TextureVars.m_Sampler, nullptr);
        vkDestroyImageView(m_Device.m_Device, m_TextureVars.m_TextureView, nullptr);
        vkDestroyImage(m_Device.m_Device, m_TextureVars.m_texture, nullptr);
        vkFreeMemory(m_Device.m_Device, m_TextureVars.m_Memory, nullptr);*/
    }

	void kTexture::createTexture(std::string filepath, VkFormat format)
	{
        std::string path1 = "../textures/";
        std::string path = path1 + filepath;
		int width, height, nrChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * 4;

		if (!pixels)
		{
			throw std::runtime_error("unable to load texture!\n");
		}

		VkBuffer staging;
		VkDeviceMemory stagingMemory;
		m_Device.createBuffers(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingMemory);

		void* data;
		vkMapMemory(m_Device.m_Device, stagingMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_Device.m_Device, stagingMemory);

		stbi_image_free(pixels);

		kImage kImage{ .m_Device = m_Device };
		kImage.createImage(width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureVars.m_texture, m_TextureVars.m_Memory);

		transitionImagelayout(format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		copyBufferToImage(staging, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		transitionImagelayout(format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(m_Device.m_Device, staging, nullptr);
		vkFreeMemory(m_Device.m_Device, stagingMemory, nullptr);

        textureImageView(format);
        createSampler();
	}
	void kTexture::transitionImagelayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
        VkCommandBuffer commandBuffer = m_SwapChain.RecordSingleCommand();

        //tähän väliin tavaraa
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_TextureVars.m_texture;
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

        m_SwapChain.endSingleCommand(commandBuffer);
	}
	void kTexture::copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height)
	{
        VkCommandBuffer commandBuffer = m_SwapChain.RecordSingleCommand();

        //soemthing here

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
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            m_TextureVars.m_texture,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        m_SwapChain.endSingleCommand(commandBuffer);
	}
	void kTexture::textureImageView(VkFormat format)
	{
        m_TextureVars.m_TextureView = m_SwapChain.createImageView(m_TextureVars.m_texture, format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
	void kTexture::createSampler()
	{
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_Device.m_PhysicalDevice, &properties);

        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_Device.m_Device, &samplerInfo, nullptr, &m_TextureVars.m_Sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
	}
    VkDescriptorImageInfo kTexture::getImageInfo()
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_TextureVars.m_TextureView;
        imageInfo.sampler = m_TextureVars.m_Sampler;
        return imageInfo;
    }
}