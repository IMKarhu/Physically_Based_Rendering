#pragma once
#include <vulkan/vulkan.h>
#include "kEntity.hpp"
#include "kDescriptors.hpp"

namespace karhu
{
    struct Frame
    {
        uint32_t CurrentFrameIndex;
        VkCommandBuffer currentCommandBuffer;
        VkDescriptorSet set;
        LveDescriptorPool& objPool;
        kEntity::MAP &entities;
    };
}