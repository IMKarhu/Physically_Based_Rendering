#include "Buffer.hpp"

#include "utils/macros.hpp"
#include "utils/utils.hpp"

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
    
    void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        /*m_Device.createBuffers(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,*/
        /*m_Buffer, m_BufferMemory);*/
        /*vkMapMemory(m_Device.m_Device, m_BufferMemory, 0, size, 0, &m_BufferMapped);*/

        VkBufferCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createinfo.size = size; //byte size of one vertices multiplied by size of vector
        createinfo.usage = usage;
        createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VK_CHECK(vkCreateBuffer(m_device, &createinfo, nullptr, &m_buffer));
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, m_buffer, &memRequirements);
        
        VkMemoryAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocinfo.allocationSize = memRequirements.size;
        allocinfo.memoryTypeIndex = utils::findMemoryType(m_phyiscalDevice, memRequirements.memoryTypeBits, properties);
        
        VK_CHECK(vkAllocateMemory(m_device, &allocinfo, nullptr, &m_bufferMemory));
        VK_CHECK(vkBindBufferMemory(m_device, m_buffer, m_bufferMemory, 0));

        vkMapMemory(m_device, m_bufferMemory, 0, size, 0, &m_bufferMapped);
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
