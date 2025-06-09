#include "kBuffer.hpp"
#include "kDevice.hpp"

namespace karhu
{
	kBuffer::kBuffer(Vulkan_Device& device)
		:m_Device(device)
	{
	}
	kBuffer::~kBuffer()
	{
		vkDestroyBuffer(m_Device.m_Device, m_Buffer, nullptr);
		vkFreeMemory(m_Device.m_Device, m_BufferMemory, nullptr);
	}
	
	void kBuffer::createBuffer(VkDeviceSize size)
	{
			m_Device.createBuffers(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_Buffer, m_BufferMemory);
			vkMapMemory(m_Device.m_Device, m_BufferMemory, 0, size, 0, &m_BufferMapped);
	}
	VkDescriptorBufferInfo kBuffer::getBufferInfo(VkDeviceSize size)
	{
		VkDescriptorBufferInfo info{};
		info.buffer = m_Buffer;
		info.offset = 0;
		info.range = size;
		return info;
	}
}