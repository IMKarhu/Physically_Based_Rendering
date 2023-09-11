#include "vertexBuffer.h"

//VertexBuffer::VertexBuffer()
//{
//}
//
//void VertexBuffer::createBuffers(VulkanRenderer& renderer)
//{
//    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//        stagingBuffer, stagingBufferMemory, renderer);
//
//    void* data;
//    vkMapMemory(getDeviceClass(renderer)->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
//    memcpy(data, vertices.data(), (size_t)bufferSize);
//    vkUnmapMemory(getDeviceClass(renderer)->getDevice(), stagingBufferMemory);
//
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//        getVertexBuffer(renderer), getDeviceBufferMemory(renderer), renderer);
//
//    copyBuffer(stagingBuffer, getVertexBuffer(renderer), bufferSize, renderer);
//
//    vkDestroyBuffer(getDeviceClass(renderer)->getDevice(), stagingBuffer, nullptr);
//    vkFreeMemory(getDeviceClass(renderer)->getDevice(), stagingBufferMemory, nullptr);
//}
//
//void VertexBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VulkanRenderer& renderer)
//{
//    VkBufferCreateInfo bufferInfo{};
//    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//    bufferInfo.size = size;
//    bufferInfo.usage = usage;
//    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//    if (vkCreateBuffer(getDeviceClass(renderer)->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to create buffer!");
//    }
//
//    VkMemoryRequirements memRequiremenets;
//    vkGetBufferMemoryRequirements(getDeviceClass(renderer)->getDevice(), buffer, &memRequiremenets);
//
//    VkMemoryAllocateInfo memAllocInfo{};
//    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    memAllocInfo.allocationSize = memRequiremenets.size;
//    memAllocInfo.memoryTypeIndex = findMemoryType(memRequiremenets.memoryTypeBits, properties, renderer);
//
//    if (vkAllocateMemory(getDeviceClass(renderer)->getDevice(), &memAllocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to allocate buffer memory!");
//    }
//
//    vkBindBufferMemory(getDeviceClass(renderer)->getDevice(), buffer, bufferMemory, 0);
//}
//
//void VertexBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VulkanRenderer& renderer)
//{
//    VkCommandBufferAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    allocInfo.commandPool = getCommandPool(renderer);
//    allocInfo.commandBufferCount = 1;
//
//    VkCommandBuffer commandBuffer;
//    vkAllocateCommandBuffers(getDeviceClass(renderer)->getDevice(), &allocInfo, &commandBuffer);
//
//    VkCommandBufferBeginInfo beginInfo{};
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//    vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//    VkBufferCopy copyRegion{};
//    copyRegion.srcOffset = 0; // Optional
//    copyRegion.dstOffset = 0; // Optional
//    copyRegion.size = size;
//    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
//
//    vkEndCommandBuffer(commandBuffer);
//
//    VkSubmitInfo submitInfo{};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &commandBuffer;
//
//    vkQueueSubmit(getDeviceClass(renderer)->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
//    vkQueueWaitIdle(getDeviceClass(renderer)->getGraphicsQueue());
//
//    vkFreeCommandBuffers(getDeviceClass(renderer)->getDevice(), getCommandPool(renderer), 1, &commandBuffer);
//}
//
//uint32_t VertexBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VulkanRenderer& renderer)
//{
//    VkPhysicalDeviceMemoryProperties memProperties;
//    vkGetPhysicalDeviceMemoryProperties(getDeviceClass(renderer)->getPhysicalDevice(), &memProperties);
//
//    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
//    {
//        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
//        {
//            return i;
//        }
//    }
//    throw std::runtime_error("failed to find memory type!");
//}
