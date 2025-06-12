#include "RenderPass.hpp"
#include "Device.hpp"

namespace karhu
{
    RenderPass::RenderPass(Device& device,
            std::vector<VkAttachmentDescription>& attachments,
            VkSubpassDescription subPassDesc,
            VkSubpassDependency subPassDep)
        : m_device(device)
    {
        createRenderPass(attachments, subPassDesc, subPassDep);
    }

    RenderPass::~RenderPass()
    {
        printf("calling Renderpass class destructor\n");
        vkDestroyRenderPass(m_device.lDevice(), m_renderPass, nullptr);
    }

    RenderPass::RenderPass(RenderPass&& other) noexcept
        : m_renderPass(other.m_renderPass), m_device(other.m_device)
    {
        other.m_renderPass = VK_NULL_HANDLE;
    }

    RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
    {
        if (this != &other)
        {
            if (m_renderPass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(m_device.lDevice(), m_renderPass, nullptr);
            }

            m_renderPass = other.m_renderPass;
            other.m_renderPass = VK_NULL_HANDLE;
        }
        return *this;
    }

    void RenderPass::createRenderPass(std::vector<VkAttachmentDescription>& attachments,
            VkSubpassDescription subPassDesc,
            VkSubpassDependency subPassDep)
    {
        VkRenderPassCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subPassDesc;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &subPassDep;

        VK_CHECK(vkCreateRenderPass(m_device.lDevice(), &createInfo, nullptr, &m_renderPass));
    }

    void RenderPass::beginRenderPass(VkFramebuffer& frameBuffer,
                    const VkExtent2D& extent,
                    std::vector<VkClearValue>& clearvalues,
                    const VkCommandBuffer& commandBuffer,
                    VkSubpassContents subPassContents)
    {
        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = m_renderPass;
        beginInfo.framebuffer = frameBuffer;
        beginInfo.renderArea.offset = {0, 0};
        beginInfo.renderArea.extent = extent;
        beginInfo.clearValueCount = static_cast<uint32_t>(clearvalues.size());
        beginInfo.pClearValues = clearvalues.data();

        vkCmdBeginRenderPass(commandBuffer, &beginInfo, subPassContents);
    }

    void RenderPass::endRenderPass(VkCommandBuffer& commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }


} // karhu namespace
