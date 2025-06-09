#pragma once

#include <stdint.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace karhu
{
    struct Frame
    {
        uint32_t currentFrameIndex;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet globalSet;
        VkDescriptorSet globalCubeSet;
        // std::vector<kEntity>& entities;
        // std::vector<kEntity>& unrealEntities;
        // std::vector<kEntity>& cubemap;
        // std::vector<std::unique_ptr<kBuffer>>& cubeBuffers;
    };
}
