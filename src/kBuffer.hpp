#pragma once
#include <vulkan/vulkan.h>

namespace karhu
{
	struct Vulkan_Device;

	class kBuffer
	{
	public:
		kBuffer(Vulkan_Device& device, VkDeviceSize size, VkBufferUsageFlags flags, VkMemoryPropertyFlags properties);
		~kBuffer();

		kBuffer(const kBuffer&) = delete;
		kBuffer& operator=(const kBuffer&) = delete;

		
		VkBuffer m_Buffer;
		VkDeviceMemory m_BufferMemory;
		void* m_BufferMapped;
		void destroy();
		void map(VkDeviceSize size, VkDeviceSize offset);
		void memcopy(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkDescriptorBufferInfo bufferInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	private:
		Vulkan_Device& m_Device;
		VkDeviceSize m_BufferSize;
	};
}