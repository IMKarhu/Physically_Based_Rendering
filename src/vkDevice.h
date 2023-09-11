#pragma once

#include "vulkan/vulkan.hpp"



namespace kge
{
	class vkInstance;

	class vkDevice
	{
	public:
		vkDevice(const vkInstance* instance, const bool& enableValidationLayers);
		~vkDevice();

		void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice pdevice);
		void createLogicalDevice(const bool& enableValidationLayers);
		bool checkDeviceExtensionSupport(VkPhysicalDevice pdevice);
		void createCommandPool();
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

		[[nodiscard]] const VkPhysicalDevice getPDevice() const { return m_PhysicalDevice; }
		[[nodiscard]] const VkDevice getDevice() const { return m_Device; }
		[[nodiscard]] const VkCommandPool getPool() const { return m_CommandPool; }
		[[nodiscard]] const VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
		[[nodiscard]] const VkQueue getPresentQueue() const { return m_PresentQueue; }
	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
		VkCommandPool m_CommandPool;

		const vkInstance* m_Instance;
	};
}
