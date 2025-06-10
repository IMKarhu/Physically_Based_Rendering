#include "Application.hpp"

#include "Image.hpp"
#include "FrameBuffer.hpp"

#include <array>

namespace karhu
{
    Application::Application()
    {
        std::vector<VkAttachmentDescription> attachments(2);
        // 0 is color 1 is depth
        attachments[0].format = m_swapChain.getSwapChainImageFormat();
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachments[1].format = m_device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subPassDesc{};
        subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPassDesc.colorAttachmentCount = 1;
        subPassDesc.pColorAttachments = &colorAttachmentRef;
        subPassDesc.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        RenderPass renderPass{ m_device, attachments, subPassDesc, dependency };
        m_renderPasses.emplace_back(std::move(renderPass));


        m_depthImage = Image(m_device.lDevice(),
                m_device.pDevice(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                m_device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        m_depthImage.createImageView(m_depthImage.getImage(),
                m_device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                VK_IMAGE_ASPECT_DEPTH_BIT,
                1);


        karhu::createFrameBuffer(m_device.lDevice(), m_framebuffers,
                m_swapChain.getSwapChainImageviews(),
                m_renderPasses[0].getRenderPass(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                1,
                false,
                m_depthImage.getImageView());

        createSyncObjects();

        // m_disneySystem.createGraphicsPipeline(m_device.lDevice(),
        //         m_swapChain.getSwapChainExtent(),
        //         std::vector<VkDescriptorSetLayout> layouts,
        //         m_renderPasses[0].getRenderPass());
    }
    Application::~Application()
    {
        for (auto frameBuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(m_device.lDevice(), frameBuffer, nullptr);
        }

        for ( size_t i = 0; i < m_commandBuffer.getMaxFramesInFlight(); i++)
        {
            vkDestroySemaphore(m_device.lDevice(), m_semaphores.m_availableSemaphores[i], nullptr);
            vkDestroySemaphore(m_device.lDevice(), m_semaphores.m_finishedSemaphores[i], nullptr);
        }
    }

    void Application::run()
    {
        update();
    }

    void Application::update()
    {
        while(!m_window->windowShouldClose())
        {
            m_window->pollEvents();
            uint32_t currentFrame = 0;
            uint32_t imageIndex = 0;
            begin(currentFrame, imageIndex);
            end(currentFrame, imageIndex);
        }
    }

    void Application::begin(uint32_t currentFrameIndex, uint32_t imageIndex)
    {
        vkWaitForFences(m_device.lDevice(), 1, &m_inFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);

        VkResult res = vkAcquireNextImageKHR(m_device.lDevice(),
                m_swapChain.getSwapchain(),
                UINT64_MAX,
                m_semaphores.m_availableSemaphores[currentFrameIndex],
                VK_NULL_HANDLE,
                &imageIndex);

        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            /*reCreateSwapChain();*/
            /*return 0;*/
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        vkResetFences(m_device.lDevice(), 1, &m_inFlightFences[currentFrameIndex]);

        m_commandBuffer.resetCommandBuffer(currentFrameIndex);

        m_commandBuffer.beginCommand(currentFrameIndex);

        VkRenderPassBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = m_renderPasses[0].getRenderPass();
        beginInfo.framebuffer = m_framebuffers[imageIndex];
        beginInfo.renderArea.offset = { 0,0 };
        beginInfo.renderArea.extent = m_swapChain.getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = { {0.0f ,0.0f ,0.0f ,1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        beginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffer.getCommandBuffer(currentFrameIndex), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = static_cast<float>(m_swapChain.getSwapChainExtent().width);
        viewPort.height = static_cast<float>(m_swapChain.getSwapChainExtent().height);
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        vkCmdSetViewport(m_commandBuffer.getCommandBuffer(currentFrameIndex), 0, 1, &viewPort);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain.getSwapChainExtent();

        vkCmdSetScissor(m_commandBuffer.getCommandBuffer(currentFrameIndex), 0, 1, &scissor);
    }

    void Application::end(uint32_t currentFrameIndex, uint32_t imageIndex)
    {
        vkCmdEndRenderPass(m_commandBuffer.getCommandBuffer(currentFrameIndex));

        m_commandBuffer.endCommandBuffer(currentFrameIndex);

        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_semaphores.m_availableSemaphores[currentFrameIndex] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &m_commandBuffer.getCommandBuffer(currentFrameIndex);
        VkSemaphore signalSemaphores[] = { m_semaphores.m_finishedSemaphores[currentFrameIndex] };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkQueueSubmit(m_device.gQueue(), 1, &submitinfo, m_inFlightFences[currentFrameIndex]));

        VkPresentInfoKHR presentinfo{};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_swapChain.getSwapchain() };
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = swapChains;
        presentinfo.pImageIndices = &imageIndex;
        presentinfo.pResults = nullptr;

        VkResult res = vkQueuePresentKHR(m_device.pQueue(), &presentinfo);

        /*if (res == VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR || m_Window->getResize())*/
        /*{*/
        /*    m_Window->setResize(false);*/
        /*    reCreateSwapChain();*/
        /*}*/
        /*else if (res != VK_SUCCESS)*/
        /*{*/
        /*    throw std::runtime_error("Failed to present swapchain image1\n");*/
        /*}*/
        currentFrameIndex = (currentFrameIndex + 1) % m_commandBuffer.getMaxFramesInFlight();
    }

    void Application::createSyncObjects()
    {
        m_semaphores.m_availableSemaphores.resize(m_commandBuffer.getMaxFramesInFlight());
        m_semaphores.m_finishedSemaphores.resize(m_commandBuffer.getMaxFramesInFlight());
        m_inFlightFences.resize(m_commandBuffer.getMaxFramesInFlight());

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo FcreateInfo{};
        FcreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FcreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_commandBuffer.getMaxFramesInFlight(); i++)
        {
            if(vkCreateSemaphore(m_device.lDevice(), &createInfo, nullptr, &m_semaphores.m_availableSemaphores[i]) != VK_SUCCESS 
                    || vkCreateSemaphore(m_device.lDevice(), &createInfo, nullptr, &m_semaphores.m_finishedSemaphores[i]) != VK_SUCCESS
                    || vkCreateFence(m_device.lDevice(), &FcreateInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error(" Failed to create Semaphores or fence!\n");
            }
        }
    }
} // namespace karhu
