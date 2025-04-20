#pragma once
#include <vulkan/vulkan.h>

namespace karhu
{
	struct Vulkan_Device;

	struct kBuffer
	{
		kBuffer(Vulkan_Device& device);
		~kBuffer();

		kBuffer(const kBuffer&) = delete;
		void operator=(const kBuffer&) = delete;

		Vulkan_Device &m_Device;
		VkBuffer m_Buffer;
		VkDeviceMemory m_BufferMemory;
		void* m_BufferMapped;
		void createBuffer(VkDeviceSize size);
		VkDescriptorBufferInfo getBufferInfo(VkDeviceSize size);

	};
}