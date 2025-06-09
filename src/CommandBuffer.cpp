#include "CommandBuffer.hpp"

/*#include "utils/macros.hpp"*/
#include "Device.hpp"

namespace karhu
{
    CommandBuffer::CommandBuffer(Device& device)
        : m_device(device)
    {
        createCommandPool();
        createCommandBuffers();
    }

    CommandBuffer::~CommandBuffer()
    {
        printf("calling CommandBuffer class destrutor\n");
        vkDestroyCommandPool(m_device.lDevice(), m_commandPool, nullptr);
    }

    void CommandBuffer::createCommandBuffers()
    {
        m_commandBuffers.resize(m_maxFramesInFlight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        VK_CHECK(vkAllocateCommandBuffers(m_device.lDevice(), &allocInfo, m_commandBuffers.data()));
    }

    void CommandBuffer::createCommandPool()
    {
        QueueFamilyIndices indices = m_device.findQueueFamilies(m_device.pDevice());

        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = indices.graphicsFamily.value();

        VK_CHECK(vkCreateCommandPool(m_device.lDevice(), &createInfo, nullptr, &m_commandPool));
    }

    void CommandBuffer::allocCommandBuffer(VkCommandBuffer& commandBuffer)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VK_CHECK(vkAllocateCommandBuffers(m_device.lDevice(), &allocInfo, &commandBuffer));
    }

    void CommandBuffer::beginCommand(VkCommandBuffer& commandBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    VkCommandBuffer CommandBuffer::recordSingleCommand()
    {
        VkCommandBuffer commandBuffer;
        allocCommandBuffer(commandBuffer);
        beginCommand(commandBuffer);
        return commandBuffer;
    }

    void CommandBuffer::endSingleCommand(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VK_CHECK(vkQueueSubmit(m_device.gQueue(), 1, &submitInfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(m_device.gQueue()));
        vkFreeCommandBuffers(m_device.lDevice(), m_commandPool, 1, &commandBuffer);
    }
} // karhu namespace
