#pragma once
#include "vulkan/vulkan.h"

#include <string>

namespace karhu
{
	struct Vulkan_Device;
	struct Vulkan_SwapChain;
	class kTexture
	{
	public:
		kTexture(Vulkan_Device& device, Vulkan_SwapChain& swapchain, std::string filepath, VkFormat format);
		~kTexture();

		struct
		{
			VkImage m_texture;
			VkDeviceMemory m_Memory;
			VkImageView m_TextureView;
			VkSampler m_Sampler;
		} m_TextureVars;

		void createTexture(std::string filepath, VkFormat format);
		void transitionImagelayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
		void textureImageView(VkFormat format);
		void createSampler();
		VkDescriptorImageInfo getImageInfo();
	private:
		Vulkan_Device& m_Device;
		Vulkan_SwapChain& m_SwapChain;
	};
}