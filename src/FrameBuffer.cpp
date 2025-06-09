#include "FrameBuffer.hpp"

#include "utils/macros.hpp"

    void karhu::createFrameBuffer(VkDevice device,
            std::vector<VkFramebuffer>& frameBuffers,
            const std::vector<VkImageView>& imageViews,
            const VkRenderPass& renderPass,
            uint32_t width,
            uint32_t height,
            uint32_t layers,
            bool isCube,
            const VkImageView& depthView)
    {
        frameBuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); i++)
        {
            std::vector<VkImageView> attachments;
            attachments.push_back(imageViews[i]);
            if (isCube == false)
            {
                attachments.push_back(depthView);
            }
            printf("size of attachments %zd \n", attachments.size());

            VkFramebufferCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.renderPass = renderPass;
            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();
            createInfo.width = width;
            createInfo.height = height;
            createInfo.layers = layers;

            VK_CHECK(vkCreateFramebuffer(device, &createInfo, nullptr, &frameBuffers[i]));
        }
    }
