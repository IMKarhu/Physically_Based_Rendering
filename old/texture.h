#pragma once

#include <iostream>

#include "buffer.h"
#include "swapChain.h"

class Buffer;
class SwapChain;

class Texture
{
	friend class GraphicsPipeLine;
public:
	Texture(Buffer* buffer, SwapChain* swapchain);
	~Texture();
	void createTextureImage(const std::string texturePath, const VkDevice& device, VkCommandPool commandPool);
	void createTextureImageViews();
	void createTextureSampler(const VkPhysicalDevice& physicalDevice, const VkDevice& device);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool commandPool);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool commandPool);
	void createDepthResources(const VkPhysicalDevice& pdevice, const VkDevice& device, VkCommandPool commandPool);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, const VkPhysicalDevice& pdevice);
	VkFormat findDepthFormat(const VkPhysicalDevice& pdevice);
	bool hasStencilComponent(VkFormat format);
	void cleanUp(const VkDevice& device);
	void cleanUpForRecreation(const VkDevice& device);

	[[nodiscard]] const VkImageView getTextureImageView() const { return m_TextureImageView; }
	[[nodiscard]] const VkSampler getTextureSampler() const { return m_TextureSampler; }
	[[nodiscard]] const VkImageView getDepthImageView() const { return m_DepthImageView; }
private:
	Buffer* m_Buffer;
	SwapChain* m_SwapChain;

	/* Class members. */
	VkImage m_TextureImage;
	VkDeviceMemory m_TextureImageMemory;
	VkImageView m_TextureImageView;
	VkSampler m_TextureSampler;
	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;

	/* Private functions. */
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, const VkDevice& device);
};