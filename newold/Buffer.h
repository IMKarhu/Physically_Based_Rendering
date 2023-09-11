#pragma once
#include "vulkan/vulkan.h"
#include "command.h"
#include <iostream>

namespace kge
{
	class Buffer
	{
	public:
        Buffer(Command* command);
		template<typename T>
		void createVkBuffer(T& vkData, VkBuffer& buffer, VkDeviceMemory& buffermemory, VkBufferUsageFlags usage, VkDevice device, VkCommandPool commandpool, const VkPhysicalDevice pdevice, const VkQueue& graphicsqueue)
		{
            VkDeviceSize bufferSize = sizeof(vkData[0]) * vkData.size();
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory, device, pdevice);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vkData.data(), (size_t)bufferSize);
            vkUnmapMemory(device, stagingBufferMemory);

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                buffer, buffermemory, device, pdevice);

            copyBuffer(stagingBuffer, buffer, bufferSize, commandpool, device, graphicsqueue);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
		}

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, const VkPhysicalDevice pdevice);
        /*VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandpool, const VkDevice& device);
        void endSingleTimeCommands(VkCommandPool commandpool, VkCommandBuffer commandBuffer, const VkDevice& device, const VkQueue& graphicsqueue);*/
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
            VkBuffer& buffer, VkDeviceMemory& bufferMemory, const VkDevice& device, const VkPhysicalDevice& pdevice);
	private:
        Command* m_Command;
        /* Private member functions. */
        
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool commandpool, const VkDevice& device, const VkQueue& graphicsqueue);
	};
}