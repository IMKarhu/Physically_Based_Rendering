#pragma once
#include "kDevice.hpp"

#include <vector>
#include <memory>


namespace karhu
{
	class Buffer
	{
    public:
        Buffer(std::shared_ptr<struct Vulkan_Device> device);

		template<typename T>
        void createVkBuffer(const std::vector<T>& data, VkBuffer& buffer, VkDeviceMemory& memory, VkBufferUsageFlags usageflags, VkCommandPool& commandPool);
        
		void createBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool);
        VkCommandBuffer beginSingleTimeCommands(VkCommandPool& commandPool);
        void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool);
	private:
        std::shared_ptr<struct Vulkan_Device> m_VkDevice;
	};
	
    template<typename T>
    void Buffer::createVkBuffer(const std::vector<T>& data, VkBuffer& buffer, VkDeviceMemory& memory, VkBufferUsageFlags usageflags, VkCommandPool& commandPool)
    {
        VkDeviceSize bufferSize = sizeof(T) * data.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* vData;
        VK_CHECK(vkMapMemory(m_VkDevice->m_Device, stagingBufferMemory, 0, bufferSize, 0, &vData));
        memcpy(vData, data.data(), (size_t)bufferSize);
        vkUnmapMemory(m_VkDevice->m_Device, stagingBufferMemory);

        createBuffers(bufferSize, usageflags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            buffer, memory);

        copyBuffer(stagingBuffer, buffer, bufferSize, commandPool);

        vkDestroyBuffer(m_VkDevice->m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_VkDevice->m_Device, stagingBufferMemory, nullptr);
    }

} //namespace karhu


/*
* VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        VK_CHECK(vkMapMemory(m_VkDevice->m_Device, stagingBufferMemory, 0, bufferSize, 0, &data));
        memcpy(data, m_Vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_VkDevice->m_Device, stagingBufferMemory);

        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffer, m_VertexBufferMemory);

        copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

        vkDestroyBuffer(m_VkDevice->m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_VkDevice->m_Device, stagingBufferMemory, nullptr);
*/