#include "kRenderer.hpp"
#include "kEntity.hpp"
#include "kCamera.hpp"

#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

namespace karhu
{
    kRenderer::kRenderer()
    {
        m_VkSwapChain.createSwapChain(m_Window->getSurface(), m_Window->getWindow());
        m_VkSwapChain.createImageViews();
        //m_GraphicsPipeline.createRenderpass(m_VkSwapChain.m_SwapChainImageFormat, findDepthFormat());
        m_VkSwapChain.createRenderPass(findDepthFormat());

        //m_DescriptorBuilder.createDescriptorSetLayout();

        //createGraphicsPipeline();
        createDepthResources();
        createFrameBuffers();

        //m_DescriptorBuilder.createDescriptorPool(2);

        createSyncObjects();
        initializeImGui();
    }
    kRenderer::~kRenderer()
    {
        printf("app destroyed\n");
        vkDestroyDescriptorPool(m_VkDevice.m_Device, m_ImguiPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        cleanUpSwapChain();
        
        for (size_t i = 0; i < m_VkSwapChain.m_MaxFramesInFlight; i++)
        {
            vkDestroySemaphore(m_VkDevice.m_Device, m_Semaphores.availableSemaphores[i], nullptr);
            vkDestroySemaphore(m_VkDevice.m_Device, m_Semaphores.finishedSemaphores[i], nullptr);
            vkDestroyFence(m_VkDevice.m_Device, m_InFlightFences[i], nullptr);
        }
        vkDestroySurfaceKHR(m_Window->getInstance(), m_Window->getSurface(), nullptr);
    }
    void kRenderer::createFrameBuffers()
    {
        m_FrameBuffers.resize(m_VkSwapChain.m_SwapChainImageViews.size());

        for (size_t i = 0; i < m_VkSwapChain.m_SwapChainImageViews.size(); i++)
        {
            std::array<VkImageView, 2> attachments = {
                m_VkSwapChain.m_SwapChainImageViews[i],
                m_DepthImageView
            };


            VkFramebufferCreateInfo createinfo{};
            createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createinfo.renderPass = m_VkSwapChain.m_RenderPass;
            createinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createinfo.pAttachments = attachments.data();
            createinfo.width = m_VkSwapChain.m_SwapChainExtent.width;
            createinfo.height = m_VkSwapChain.m_SwapChainExtent.height;
            createinfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_VkDevice.m_Device, &createinfo, nullptr, &m_FrameBuffers[i]));
        }
    }

    void kRenderer::recordCommandBuffer(glm::vec3 position, glm::vec3 lightPos, glm::vec4 lightColor, Frame& frameInfo)
    {
        vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.getPipeline());
        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_GraphicsPipeline.getPipelineLayout(),
            0,
            1,
            &frameInfo.globalSet,
            0,
            nullptr);

        for (auto& entity : frameInfo.entities)
        {
            

            
            vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_GraphicsPipeline.getPipelineLayout(),
                1,
                1,
                &entity.m_DescriptorSet,
                0,
                nullptr);

            ObjPushConstant objConstant{};
            objConstant.model = entity.getTransformMatrix();
            vkCmdPushConstants(frameInfo.commandBuffer,
                m_GraphicsPipeline.getPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(ObjPushConstant),
                &objConstant);

            pushConstants cameraConstants{};
            cameraConstants.cameraPosition = position;
            cameraConstants.lightPosition = vars.m_LightPosition;
            cameraConstants.lighColor = lightColor;
            cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, vars.m_Metalness, vars.m_Roughness);
            vkCmdPushConstants(frameInfo.commandBuffer,
                m_GraphicsPipeline.getPipelineLayout(),
                VK_SHADER_STAGE_FRAGMENT_BIT,
                64,
                sizeof(pushConstants),
                &cameraConstants);


            entity.getModel()->bind(frameInfo.commandBuffer);
            entity.getModel()->draw(frameInfo.commandBuffer);
        }
    }

    VkCommandBuffer kRenderer::beginRecordCommandBuffer(uint32_t currentFrameIndex, uint32_t index)
    {
        VkCommandBuffer commandBuffer = m_VkSwapChain.m_CommandBuffers[currentFrameIndex];
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = 0;
        info.pInheritanceInfo = nullptr;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &info));

        VkRenderPassBeginInfo renderpassInfo{};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassInfo.renderPass = m_VkSwapChain.m_RenderPass;
        renderpassInfo.framebuffer = m_FrameBuffers[index];
        renderpassInfo.renderArea.offset = { 0,0 };
        renderpassInfo.renderArea.extent = m_VkSwapChain.m_SwapChainExtent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = { {0.01f ,0.01f ,0.01f ,1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderpassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.getPipeline());

        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = static_cast<float>(m_VkSwapChain.m_SwapChainExtent.width);
        viewPort.height = static_cast<float>(m_VkSwapChain.m_SwapChainExtent.height);
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewPort);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_VkSwapChain.m_SwapChainExtent;

        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        return commandBuffer;
    }

    void kRenderer::endRecordCommandBuffer(VkCommandBuffer commandBuffer)
    {
        ImGui::EndFrame();
        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void kRenderer::createSyncObjects()
    {
        m_Semaphores.availableSemaphores.resize(m_VkSwapChain.m_MaxFramesInFlight);
        m_Semaphores.finishedSemaphores.resize(m_VkSwapChain.m_MaxFramesInFlight);
        m_InFlightFences.resize(m_VkSwapChain.m_MaxFramesInFlight);

        VkSemaphoreCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceinfo{};
        fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_VkSwapChain.m_MaxFramesInFlight; i++)
        {
            if (vkCreateSemaphore(m_VkDevice.m_Device, &createinfo, nullptr, &m_Semaphores.availableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_VkDevice.m_Device, &createinfo, nullptr, &m_Semaphores.finishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_VkDevice.m_Device, &fenceinfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create Semaphores!");
            }
        }

    }

    void kRenderer::createUniformBuffers(kBuffer& buffer)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

            m_VkDevice.createBuffers(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                buffer.m_Buffer, buffer.m_BufferMemory);
            vkMapMemory(m_VkDevice.m_Device, buffer.m_BufferMemory, 0, bufferSize, 0, &buffer.m_BufferMapped);

    }

    void kRenderer::updateUBOs(std::vector<std::unique_ptr<kBuffer>>& buffers, kCamera& camera)
    {
        for (auto& buffer : buffers)
        {
            UniformBufferObject ubo{};
           
            ubo.view = camera.getView();
            ubo.proj = camera.getProjection();
            ubo.proj[1][1] *= -1;

            memcpy(buffer->m_BufferMapped, &ubo, sizeof(ubo));
        }
    }

    void kRenderer::updateObjBuffers(std::vector<kBuffer>& buffers, kEntity& entity)
    {
        for (auto& buffer : buffers)
        {
            ObjBuffer objBuffer{};
            objBuffer.model = entity.getTransformMatrix();

            memcpy(buffer.m_BufferMapped, &objBuffer, sizeof(objBuffer));
        }
    }

    VkCommandBuffer kRenderer::beginFrame(uint32_t m_currentFrameIndex, uint32_t imageIndex)
    {
        vkWaitForFences(m_VkDevice.m_Device, 1, &m_InFlightFences[m_currentFrameIndex], VK_TRUE, UINT64_MAX);

        //uint32_t imageIndex;
        VkResult res = vkAcquireNextImageKHR(m_VkDevice.m_Device, m_VkSwapChain.m_SwapChain, UINT64_MAX, m_Semaphores.availableSemaphores[m_currentFrameIndex], VK_NULL_HANDLE, &imageIndex);
        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            reCreateSwapChain();
            return 0;
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        vkResetFences(m_VkDevice.m_Device, 1, &m_InFlightFences[m_currentFrameIndex]);

        VkCommandBuffer commandBuffer = m_VkSwapChain.m_CommandBuffers[m_currentFrameIndex];

        vkResetCommandBuffer(commandBuffer, 0);

        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = 0;
        info.pInheritanceInfo = nullptr;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &info));

        VkRenderPassBeginInfo renderpassInfo{};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassInfo.renderPass = m_VkSwapChain.m_RenderPass;
        renderpassInfo.framebuffer = m_FrameBuffers[imageIndex];
        renderpassInfo.renderArea.offset = { 0,0 };
        renderpassInfo.renderArea.extent = m_VkSwapChain.m_SwapChainExtent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = { {0.0f ,0.0f ,0.0f ,1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderpassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.getPipeline());

        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = static_cast<float>(m_VkSwapChain.m_SwapChainExtent.width);
        viewPort.height = static_cast<float>(m_VkSwapChain.m_SwapChainExtent.height);
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewPort);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_VkSwapChain.m_SwapChainExtent;

        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        return commandBuffer;
    }

    void kRenderer::endFrame(uint32_t m_currentFrameIndex, uint32_t imageIndex, VkCommandBuffer commandBuffer)
    {
       // ImGui::EndFrame();
        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));


        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_Semaphores.availableSemaphores[m_currentFrameIndex] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &m_VkSwapChain.m_CommandBuffers[m_currentFrameIndex];
        VkSemaphore signalSemaphores[] = { m_Semaphores.finishedSemaphores[m_currentFrameIndex] };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkQueueSubmit(m_VkDevice.m_GraphicsQueue, 1, &submitinfo, m_InFlightFences[m_currentFrameIndex]));

        VkPresentInfoKHR presentinfo{};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_VkSwapChain.m_SwapChain };
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = swapChains;
        presentinfo.pImageIndices = &imageIndex;
        presentinfo.pResults = nullptr;

        VkResult res = vkQueuePresentKHR(m_VkDevice.m_PresentQueue, &presentinfo);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR || m_Window->getResize())
        {
            m_Window->setResize(false);
            reCreateSwapChain();
        }
        else if (res != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swapchain image1\n");
        }
        m_currentFrameIndex = (m_currentFrameIndex + 1) % m_VkSwapChain.m_MaxFramesInFlight;
    }

    void kRenderer::renderImguiLayer(VkCommandBuffer commandBuffer, Frame& frameInfo, float dt)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        auto entPos = frameInfo.entities[1].getPosition();
        ImGui::NewFrame();

       // ImGui::ShowDemoWindow();
        ImGui::Begin("Controls");
        ImGui::SliderFloat("Metalness", &vars.m_Metalness, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &vars.m_Roughness, 0.0f, 1.0f);
        ImGui::SliderFloat3("lightPosition", glm::value_ptr(vars.m_LightPosition), -50.0f, 50.0f);

        ImGui::SliderFloat3("objPosition", glm::value_ptr(entPos), -5.0f, 200.0f);
        ImGui::End();

        frameInfo.entities[1].setPosition(entPos);

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void kRenderer::createGraphicsPipeline(std::vector<VkDescriptorSetLayout> layouts)
    {
        GraphicsPipelineStruct pipelineStruct{};
        pipelineStruct.viewportWidth = m_VkSwapChain.m_SwapChainExtent.width;
        pipelineStruct.viewportheight = m_VkSwapChain.m_SwapChainExtent.height;
        pipelineStruct.scissor.extent = m_VkSwapChain.m_SwapChainExtent;
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = layouts.data();

        m_GraphicsPipeline.createPipeline(pipelineStruct, "../shaders/vertexShader.spv", "../shaders/fragmentShader.spv");
    }

    void kRenderer::createDepthResources()
    {
        VkFormat depthFormat = findDepthFormat();
        createImage(m_VkSwapChain.m_SwapChainExtent.width,
            m_VkSwapChain.m_SwapChainExtent.height,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_DepthImage,
            m_DepthImageMemory);
        m_DepthImageView = m_VkSwapChain.createImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    VkFormat kRenderer::findDepthFormat()
    {
        return m_VkDevice.findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool kRenderer::hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void kRenderer::createImage(uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateImage(m_VkDevice.m_Device, &imageInfo, nullptr, &image));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_VkDevice.m_Device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_VkDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_VkDevice.m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_VkDevice.m_Device, image, imageMemory, 0);
    }

    void kRenderer::cleanUpSwapChain()
    {
        vkDestroyImageView(m_VkDevice.m_Device, m_DepthImageView, nullptr);
        vkDestroyImage(m_VkDevice.m_Device, m_DepthImage, nullptr);
        vkFreeMemory(m_VkDevice.m_Device, m_DepthImageMemory, nullptr);
        for (auto framebuffer : m_FrameBuffers)
        {
            vkDestroyFramebuffer(m_VkDevice.m_Device, framebuffer, nullptr);
        }
        for (auto imageView : m_VkSwapChain.m_SwapChainImageViews)
        {
            vkDestroyImageView(m_VkDevice.m_Device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_VkDevice.m_Device, m_VkSwapChain.m_SwapChain, nullptr);
    }

    void kRenderer::reCreateSwapChain()
    {
        /*int width = 0;
        int height = 0;
        m_Window->getFrameBufferSize(m_Window->getWindow(), width, height);
        while (width == 0 || height == 0)
        {
            printf("hello2\n");
            if (m_Window->shouldClose())
            {
                return;
            }
            m_Window->getFrameBufferSize(m_Window->getWindow(), width, height);
            m_Window->waitEvents();
        }*/

        VK_CHECK(vkDeviceWaitIdle(m_VkDevice.m_Device));

        cleanUpSwapChain();

        m_VkSwapChain.createSwapChain(m_Window->getSurface(), m_Window->getWindow());
        m_VkSwapChain.createImageViews();
        createDepthResources();
        createFrameBuffers();
    }

    //poor implementation but its from imgui examples
    void kRenderer::initializeImGui()
    {
        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(poolSizes);
        pool_info.pPoolSizes = poolSizes;

        VK_CHECK(vkCreateDescriptorPool(m_VkDevice.m_Device, &pool_info, nullptr, &m_ImguiPool));


        //initialize imgui

        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForVulkan(m_Window->getWindow(), true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = m_Window->getInstance();
        initInfo.PhysicalDevice = m_VkDevice.m_PhysicalDevice;
        initInfo.Device = m_VkDevice.m_Device;
        initInfo.Queue = m_VkDevice.m_GraphicsQueue;
        initInfo.DescriptorPool = m_ImguiPool;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.RenderPass = m_VkSwapChain.m_RenderPass;

        ImGui_ImplVulkan_Init(&initInfo);

        VkCommandBuffer commandBuffer = m_VkSwapChain.RecordSingleCommand();
        ImGui_ImplVulkan_CreateFontsTexture();
        m_VkSwapChain.endSingleCommand(commandBuffer);

        vkDeviceWaitIdle(m_VkDevice.m_Device);
        ImGui_ImplVulkan_DestroyFontsTexture();
        
    }
}