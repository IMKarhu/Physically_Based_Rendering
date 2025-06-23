#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <memory>


namespace karhu
{
    enum FramebufferType
    {
        Normal,
        Cube,
        BRDFLUT
    };

    void createFrameBuffer(VkDevice device,
            std::vector<VkFramebuffer>& frameBuffers,
            const std::vector<VkImageView>& imageViews,
            const VkRenderPass& renderPass,
            uint32_t width,
            uint32_t height,
            uint32_t layers,
            bool isCube = false,
            const VkImageView& depthView = nullptr);

    void createFrameBuffer1(VkDevice device,
            VkFramebuffer& frameBuffer,
            const VkImageView& imageView,
            const VkRenderPass& renderPass,
            uint32_t width,
            uint32_t height,
            uint32_t layers,
            bool isCube = false,
            const VkImageView& depthView = nullptr);

} // karhu namespace
