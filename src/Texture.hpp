#pragma once

#include "Image.hpp"
#include "vulkan/vulkan.h"

#include <string>
#include <vector>
#include <memory>

namespace karhu
{
    class Device;
    class CommandBuffer;
    class Texture
    {
    public:
        Texture(Device& device);
        virtual ~Texture() = 0;

        //delete copy constructors
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& other) noexcept
            : m_image(std::move(other.m_image))
            , m_device(other.m_device) {}
        Texture& operator =(Texture&& other) noexcept
        {
            if (this != &other)
            {
                m_image = std::move(other.m_image);
            }
            return *this;
        }
        
        // struct
        // {
        //     VkImage m_texture;
        //     VkDeviceMemory m_Memory;
        //     VkImageView m_TextureView;
        //     std::vector<VkImageView> m_ImageViews; //for cubemap
        //     VkSampler m_Sampler;
        // } m_TextureVars;
        
        // void createTexture(std::string filepath, VkFormat format);
        // void createHDRTexture(std::string filepath, VkFormat format);
        // void transitionImagelayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        // void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
        // void copyBufferToImageCube(VkBuffer buffer, uint32_t width, uint32_t height);
        // void textureImageView(VkFormat format, uint32_t layerCount, bool cubemap);
        // void createSampler(VkSamplerAddressMode addressMode);
        // VkDescriptorImageInfo getImageInfo();
        const VkImage& getImage() const;
        const VkImageView& getImageView() const;
    protected:
        std::unique_ptr<Image> m_image;
    private:
        Device& m_device;
    };

    class NTexture : public Texture
    {
        public:
            NTexture(Device& device, CommandBuffer& commandBuffer, std::string filePath, VkFormat format);
            ~NTexture() override;

            NTexture(const NTexture&) = delete;
            NTexture& operator=(const NTexture&) = delete;

            NTexture(NTexture&&) noexcept;
            NTexture& operator=(NTexture&&) noexcept;

            void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
            void copyBufferToImage(VkBuffer buffer);
            void createSampler(VkSamplerAddressMode addressMode);
            VkDescriptorImageInfo getImageInfo();
        private:
            int m_width = 0;
            int m_height = 0;
            int m_nrChannels = 0;
            VkDeviceSize m_imageSize;
            VkSampler m_sampler;
            Device& m_device;
            CommandBuffer& m_commandBuffer;
    };

    class CubeTexture : public Texture
    {
        public:
            CubeTexture(Device& device, std::string filePath, VkFormat format);
            ~CubeTexture() override;
        private:
            int m_width = 0;
            int m_height = 0;
            int m_nrChannels = 0;
            VkDeviceSize m_imageSize;
            VkSampler m_sampler;
            Device& m_device;
    };
}
