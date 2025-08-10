#pragma once

#include "Entity.hpp"

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
        /*VkDescriptorSet globalCubeSet;*/
        Entity& camera;
        std::vector<Entity>& entities;
        std::vector<Entity>& spheres;
        std::vector<Entity>& unrealEntities;
        std::vector<Entity>& unrealSpheres;

    };
}
