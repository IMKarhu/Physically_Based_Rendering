#pragma once
#include "vulkan/vulkan.h"

#include <string>
#include <vector>

namespace karhu
{
    class Device;
    struct Vulkan_SwapChain;
    class Texture
    {
    public:
        Texture(Device& device, Vulkan_SwapChain& swapchain, std::string filepath, VkFormat format, bool hdr = false);
        ~Texture();
        
        struct
        {
            VkImage m_texture;
            VkDeviceMemory m_Memory;
            VkImageView m_TextureView;
            std::vector<VkImageView> m_ImageViews; //for cubemap
            VkSampler m_Sampler;
        } m_TextureVars;
        
        void createTexture(std::string filepath, VkFormat format);
        void createHDRTexture(std::string filepath, VkFormat format);
        void transitionImagelayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
        void copyBufferToImageCube(VkBuffer buffer, uint32_t width, uint32_t height);
        void textureImageView(VkFormat format, uint32_t layerCount, bool cubemap);
        void createSampler(VkSamplerAddressMode addressMode);
        VkDescriptorImageInfo getImageInfo();
    private:
        Device& m_device;
        Vulkan_SwapChain& m_SwapChain;
    };
}
