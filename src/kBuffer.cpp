#include "kBuffer.hpp"

namespace karhu
{
    Buffer::Buffer(std::shared_ptr<struct Vulkan_Device> device)
        : m_VkDevice(std::move(device))
    {
    }

    void Buffer::createBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
        VkBufferCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createinfo.size = size; //byte size of one vertices multiplied by size of vector
        createinfo.usage = usage;
        createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(m_VkDevice->m_Device, &createinfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_VkDevice->m_Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocinfo.allocationSize = memRequirements.size;
        /*VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/
        allocinfo.memoryTypeIndex = m_VkDevice->findMemoryType(memRequirements.memoryTypeBits, properties);

        VK_CHECK(vkAllocateMemory(m_VkDevice->m_Device, &allocinfo, nullptr, &bufferMemory));
        //  printf("Hello\n");
        VK_CHECK(vkBindBufferMemory(m_VkDevice->m_Device, buffer, bufferMemory, 0));
	}

    void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer, commandPool);
    }
    VkCommandBuffer Buffer::beginSingleTimeCommands(VkCommandPool& commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VK_CHECK(vkAllocateCommandBuffers(m_VkDevice->m_Device, &allocInfo, &commandBuffer));

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        return commandBuffer;
    }
    void Buffer::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VK_CHECK(vkQueueSubmit(m_VkDevice->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(m_VkDevice->m_GraphicsQueue));
        vkFreeCommandBuffers(m_VkDevice->m_Device, commandPool, 1, &commandBuffer);
    }
}

