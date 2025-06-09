#include "Buffer.hpp"

namespace karhu
{
    Buffer::Buffer()
    {
    }
    Buffer::~Buffer()
    {
        /*vkDestroyBuffer(m_Device.m_Device, m_Buffer, nullptr);*/
        /*vkFreeMemory(m_Device.m_Device, m_BufferMemory, nullptr);*/
    }
    
    void Buffer::createBuffer(VkDeviceSize size)
    {
        /*m_Device.createBuffers(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,*/
        /*m_Buffer, m_BufferMemory);*/
        /*vkMapMemory(m_Device.m_Device, m_BufferMemory, 0, size, 0, &m_BufferMapped);*/
    }
    VkDescriptorBufferInfo Buffer::getBufferInfo(VkDeviceSize size)
    {
        VkDescriptorBufferInfo info{};
        info.buffer = m_buffer;
        info.offset = 0;
        info.range = size;
        return info;
    }
}
