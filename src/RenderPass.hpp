#pragma once

#include "vulkan/vulkan.h"

#include <vector>

namespace karhu
{
    class Device;
    class RenderPass
    {
        public:
            RenderPass(Device& device,
                    std::vector<VkAttachmentDescription>& attachments,
                    VkSubpassDescription subPassDesc,
                    VkSubpassDependency subPassDep);

            ~RenderPass();

            RenderPass(const RenderPass&) = delete;
            void operator=(const RenderPass&) = delete;
            RenderPass(RenderPass&&) noexcept;
            RenderPass& operator=(RenderPass&&) noexcept;

            void createRenderPass(std::vector<VkAttachmentDescription>& attachments,
                    VkSubpassDescription subPassDesc,
                    VkSubpassDependency subPassDep);

            void beginRenderPass(VkFramebuffer& frameBuffer,
                    VkExtent2D& extent,
                    std::vector<VkClearValue>& clearvalues,
                    VkCommandBuffer& commandBuffer,
                    VkSubpassContents& subPassContents);

            void endRenderPass(VkCommandBuffer& commandBuffer);

            const VkRenderPass& getRenderPass() const { return m_renderPass; }
        private:
            VkRenderPass m_renderPass;

            Device& m_device;
    };
}
