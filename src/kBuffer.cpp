#include "kBuffer.hpp"
#include "kDevice.hpp"
#include <cstring>

namespace karhu
{
	kBuffer::kBuffer(Vulkan_Device& device, VkDeviceSize size, VkBufferUsageFlags flags, VkMemoryPropertyFlags properties)
		:m_Device(device)
		,m_BufferSize(size)
	{
		m_Device.createBuffers(m_BufferSize, flags, properties,
			m_Buffer, m_BufferMemory);
	}
	kBuffer::~kBuffer()
	{
	}
	void kBuffer::destroy()
	{
		vkDestroyBuffer(m_Device.m_Device, m_Buffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_BufferMemory, nullptr);
	}
	void kBuffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		vkMapMemory(m_Device.m_Device, m_BufferMemory, offset, size, 0, &m_BufferMapped);
	}
	void kBuffer::memcopy(void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		memcpy(m_BufferMapped, &data, size);
	}
	VkDescriptorBufferInfo kBuffer::bufferInfo(VkDeviceSize size, VkDeviceSize offset)
	{
		return VkDescriptorBufferInfo{m_Buffer,
									  offset,
									  size
		};
	}
}