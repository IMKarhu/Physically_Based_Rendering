#pragma once
#include <vulkan/vulkan.h>
#include "kBuffer.hpp"

namespace karhu
{
	struct Vulkan_Device;

	class Texture
	{
	public:
		explicit Texture(Vulkan_Device& device);
		~Texture();

		Texture(const Texture&) = delete;
		Texture(Texture&& other) = delete;

		Texture& operator=(Texture&) = delete;
		Texture& operator=(Texture&&) = delete;
		//void loadFile(std::string fileName, VkFormat format);
		void createTexture(const char* fileName, VkCommandPool& commandPool);
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
			VkImage& image, VkDeviceMemory& imageMemory);
		void createTextureImageView();
		VkImageView createImageview(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void createSampler();
		void transitionImagelayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool& commandPool);
		void copyBufferToImage(VkBuffer vbuffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool& commandPool);

		VkImageView imageView() const { return m_ImageView; }
		VkSampler textureSampler() const { return m_Sampler; }
	private:
		Vulkan_Device& m_VkDevice;
		Buffer buffer{ m_VkDevice };
		VkImage m_TextureImage = VK_NULL_HANDLE;
		VkDeviceMemory m_TextureMemory = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSet;
		uint32_t m_Width;
		uint32_t m_Height;
	};
} // karhu