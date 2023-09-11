#pragma once
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "vulkan/vulkan.h"

#include "../command.h"
#include "../vkSwapChain.h"
#include "../Buffer.h"

#include <iostream>
#include <vector>


namespace kge
{
	struct Texture
	{
		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
		VkSampler m_TextureSampler;
		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;
	};

	class Model
	{
	public:
		Model(Command* command, vkSwapChain* swapchain, Buffer* buffer);
		~Model();

		void createTextureImage(const VkDevice& device, const VkPhysicalDevice& pdevice, const VkQueue& graphicsqueue, const std::string texturePath);
		void createTextureImageView(const VkDevice& device);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool commandPool, const VkDevice& device, const VkQueue& graphicsqueue);
		void createTextureSampler(const VkPhysicalDevice& physicalDevice, const VkDevice& device);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool commandPool, const VkDevice& device, const VkQueue& graphisqueue, const VkPhysicalDevice& pdevice);
		void createDepthResources(const VkPhysicalDevice& pdevice, const VkDevice& device, VkCommandPool commandPool, const VkQueue& graphicsqueue);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, const VkPhysicalDevice& pdevice);
		VkFormat findDepthFormat(const VkPhysicalDevice& pdevice);
		bool hasStencilComponent(VkFormat format);

		Texture m_Texture;
	private:
		Command* m_Command;
		vkSwapChain* m_SwapChain;
		Buffer* m_Buffer;
		
		

		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, const VkDevice& device, const VkPhysicalDevice& pdevice);
	};

	
}