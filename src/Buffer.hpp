#pragma once
#include <vulkan/vulkan.h>

namespace karhu
{
    
    struct Buffer
    {
        Buffer();
        ~Buffer();
        
        Buffer(const Buffer&) = delete;
        void operator=(const Buffer&) = delete;
        
        VkBuffer m_buffer;
        VkDeviceMemory m_bufferMemory;
        void* m_bufferMapped;
        VkDevice m_device;
        VkPhysicalDevice m_phyiscalDevice;
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        VkDescriptorBufferInfo getBufferInfo(VkDeviceSize size);
    };
}
