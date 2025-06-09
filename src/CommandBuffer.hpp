#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace karhu
{
    class Device;

    class CommandBuffer
    {
        public:
            CommandBuffer(Device& device);
            ~CommandBuffer();

            CommandBuffer(const CommandBuffer&) = delete;
            void operator=(const CommandBuffer&) = delete;
            CommandBuffer(CommandBuffer&&) = delete;
            CommandBuffer& operator=(CommandBuffer&&) = delete;

            void createCommandBuffers();
            void createCommandPool();
            void allocCommandBuffer(VkCommandBuffer& commandBuffer);
            void beginCommand(VkCommandBuffer& commandBuffer);
            VkCommandBuffer recordSingleCommand();
            void endSingleCommand(VkCommandBuffer commandBuffer);
        private:
            VkCommandPool m_commandPool = VK_NULL_HANDLE;
            std::vector<VkCommandBuffer> m_commandBuffers;
            const int m_maxFramesInFlight = 2;

            Device& m_device;
    };
} // karhu namespace
