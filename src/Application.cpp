#include "Application.hpp"

#include "Image.hpp"
#include "FrameBuffer.hpp"
#include "Camera.hpp"
#include "keyboardMovement.hpp"
#include "frame.hpp"

#include <array>
#include <chrono>

namespace karhu
{
    Application::Application()
    {
        
        printf("end of application constructor\n");
    }
    Application::~Application()
    {
        cleanUpBeforeReCreate();

        for ( size_t i = 0; i < m_commandBuffer.getMaxFramesInFlight(); i++)
        {
            vkDestroySemaphore(m_device.lDevice(), m_semaphores.m_availableSemaphores[i], nullptr);
            vkDestroySemaphore(m_device.lDevice(), m_semaphores.m_finishedSemaphores[i], nullptr);
        }
    }

    void Application::run()
    {

        printf("start of run\n");
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

        printf("before renderpass creation");
        // RenderPass renderPass{ m_device, attachments, subPassDesc, dependency };
        m_renderPasses.emplace_back(m_device, attachments, subPassDesc, dependency);
        printf("after renderpass creation");


        m_depthImage = Image(m_device.lDevice(),
                m_device.pDevice(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                m_device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        /*m_depthImage.createImageView(m_depthImage.getImage(),*/
        /*        m_device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },*/
        /*            VK_IMAGE_TILING_OPTIMAL,*/
        /*            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),*/
        /*        VK_IMAGE_ASPECT_DEPTH_BIT,*/
        /*        1);*/


        karhu::createFrameBuffer(m_device.lDevice(), m_framebuffers,
                m_swapChain.getSwapChainImageviews(),
                m_renderPasses[0].getRenderPass(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                1,
                false,
                m_depthImage.getImageView());

        createSyncObjects();

        auto model = std::make_shared<Model>(m_device, m_commandBuffer, "../models/DamagedHelmet.gltf");

        /*Probably shouldn't be here but it'll work for now..*/
        m_builder.addPoolElement(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
        m_pool = m_builder.createDescriptorPool(2);

        m_builder.bind(m_bindings, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        m_layout = m_builder.createDescriptorSetLayout(m_bindings);

        std::vector<std::unique_ptr<Buffer>> uboBuffers(2);
        for (size_t i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<Buffer>();
            uboBuffers[i]->m_device = m_device.lDevice();
            uboBuffers[i]->m_phyiscalDevice = m_device.pDevice();
            uboBuffers[i]->createBuffer(sizeof(UniformBufferObject),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        m_builder.allocateDescriptor(m_set, m_layout, m_pool);
        m_builder.writeBuffer(m_set, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboBuffers[0]->getBufferInfo(sizeof(UniformBufferObject)), 0);
        m_builder.fillWritesMap(0);
        m_builder.createDescriptorSets(m_layout, m_pool);

        auto ent1 = Entity::createEntity();
        ent1.setModel(model);
        ent1.setPosition({0.0f, 0.0f, -5.0f});
        ent1.setScale({1.0f, 1.0f, 1.0f});
        ent1.setRotation({90.0f, 0.0f, 0.0f});

        m_entities[Disney].push_back(std::move(ent1));

        m_disneySystem.createDescriptors(m_entities[Disney]);
        m_disneySystem.createGraphicsPipeline(m_device.lDevice(),
                m_swapChain.getSwapChainExtent(),
                m_layout,
                m_renderPasses[0].getRenderPass());


        update(uboBuffers);
    }

    void Application::update(std::vector<std::unique_ptr<Buffer>>& gBuffers)
    {
        auto cameraEntity = Entity::createEntity();
        cameraEntity.setPosition({0.0f, 0.0f, -20.0});
        Camera camera{};
        keyboardMovement movement{};
        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!m_window->windowShouldClose())
        {
            m_window->pollEvents();
            uint32_t imageIndex = 0;
            auto startTime = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::chrono::seconds::period>(startTime - currentTime).count();
            currentTime = startTime;

            camera.update(dt);
            movement.update(m_window->getWindow(),
                    dt,
                    camera,
                    m_swapChain.getSwapChainExtent().width,
                    m_swapChain.getSwapChainExtent().height);
            camera.setView(cameraEntity.getPosition(),
                    cameraEntity.getRotation(),
                    glm::vec3(0.0f, 1.0f, 0.0f));
            camera.setPerspective(glm::radians(45.0f),
                    m_swapChain.getSwapChainExtent().width / (float)m_swapChain.getSwapChainExtent().height,
                    0.1f, 100.0f);


            begin(m_currentFrame, imageIndex);

            Frame frameInfo
            {
                m_currentFrame,
                m_commandBuffer.getCommandBuffer(m_currentFrame),
                m_set,
                cameraEntity,
                m_entities[Disney]
            };

            m_disneySystem.renderEntities(frameInfo);

            updateBuffers(gBuffers, camera);

            for (auto& entity : m_entities[Disney])
            {
                entity.updateBuffer();
            }

            end(m_currentFrame, imageIndex);
        }
        VK_CHECK(vkDeviceWaitIdle(m_device.lDevice()));
    }

    void Application::begin(uint32_t currentFrameIndex, uint32_t imageIndex)
    {
        static uint32_t imageIndexx = 0;
        vkWaitForFences(m_device.lDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        VkResult res = vkAcquireNextImageKHR(m_device.lDevice(),
                m_swapChain.getSwapchain(),
                UINT64_MAX,
                m_semaphores.m_availableSemaphores[m_currentFrame],
                VK_NULL_HANDLE,
                &imageIndex);

        if (imageIndexx == 0)
            std::cout << "res:" << res << std::endl;

        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            printf("out of date");
            reCreateSwapChain();
            return;
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        vkResetFences(m_device.lDevice(), 1, &m_inFlightFences[m_currentFrame]);

        m_commandBuffer.resetCommandBuffer(m_currentFrame);

        m_commandBuffer.beginCommand(m_currentFrame);

        // VkRenderPassBeginInfo beginInfo{};
        // beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        // beginInfo.renderPass = m_renderPasses[0].getRenderPass();
        // beginInfo.framebuffer = m_framebuffers[imageIndex];
        // beginInfo.renderArea.offset = { 0,0 };
        // beginInfo.renderArea.extent = m_swapChain.getSwapChainExtent();

        std::vector<VkClearValue> clearValues;
        clearValues.resize(2);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

            // .color = { {0.0f ,0.0f ,0.0f ,1.0f} },
            // .depthStencil = { 1.0f, 0 }
        // beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        // beginInfo.pClearValues = clearValues.data();

        // vkCmdBeginRenderPass(m_commandBuffer.getCommandBuffer(m_currentFrame), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_renderPasses[0].beginRenderPass(m_framebuffers[imageIndex], m_swapChain.getSwapChainExtent(), clearValues, m_commandBuffer.getCommandBuffer(m_currentFrame), VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = static_cast<float>(m_swapChain.getSwapChainExtent().width);
        viewPort.height = static_cast<float>(m_swapChain.getSwapChainExtent().height);
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        vkCmdSetViewport(m_commandBuffer.getCommandBuffer(m_currentFrame), 0, 1, &viewPort);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChain.getSwapChainExtent();

        vkCmdSetScissor(m_commandBuffer.getCommandBuffer(m_currentFrame), 0, 1, &scissor);

        imageIndexx++;
    }

    void Application::end(uint32_t currentFrameIndex, uint32_t imageIndex)
    {
        vkCmdEndRenderPass(m_commandBuffer.getCommandBuffer(m_currentFrame));

        m_commandBuffer.endCommandBuffer(m_currentFrame);

        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_semaphores.m_availableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &m_commandBuffer.getCommandBuffer(m_currentFrame);
        VkSemaphore signalSemaphores[] = { m_semaphores.m_finishedSemaphores[m_currentFrame] };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkQueueSubmit(m_device.gQueue(), 1, &submitinfo, m_inFlightFences[m_currentFrame]));

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

        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_window->resized())
        {
            // printf("out of date or suboptimal or window was resized\n");
            std::cout << "res:" << res << std::endl;
            m_window->setResized(false);
            reCreateSwapChain();
        }
        else if (res != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swapchain image1\n");
        }
        m_currentFrame = (m_currentFrame + 1) % m_commandBuffer.getMaxFramesInFlight();
    }

    void Application::updateBuffers(std::vector<std::unique_ptr<Buffer>>& gBuffers, Camera& camera)
    {
        for( auto& buffer : gBuffers)
        {
            UniformBufferObject obj{};

            obj.view = camera.getView();
            obj.proj = camera.getProjection();
            obj.proj[1][1] *= -1;

            memcpy(buffer->m_bufferMapped, &obj, sizeof(obj));
        }
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

    void Application::cleanUpBeforeReCreate()
    {
        /*vkDestroyImageView(m_device.lDevice(), m_depthImage.getImageView(), nullptr);*/
        /*vkDestroyImage(m_device.lDevice(), m_depthImage.getImage(), nullptr);*/
        for (auto frameBuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(m_device.lDevice(), frameBuffer, nullptr);
        }
        for (auto view : m_swapChain.getSwapChainImageviews())
        {
            vkDestroyImageView(m_device.lDevice(), view, nullptr);
        }
        vkDestroySwapchainKHR(m_device.lDevice(), m_swapChain.getSwapchain(), nullptr);
    }

    void Application::reCreateSwapChain()
    {
        int width = 0;
        int height = 0;
        m_window->frameBufferSize(m_window->getWindow(), width, height);

        while (width == 0 || height == 0)
        {
            if (m_window->windowShouldClose())
            {
                return;
            }
            m_window->frameBufferSize(m_window->getWindow(), width, height);
            m_window->waitEvents();
        }

        VK_CHECK(vkDeviceWaitIdle(m_device.lDevice()));

        cleanUpBeforeReCreate();

        printf("size of framebuffers: %lld\n", m_framebuffers.size());
        printf("size of imageviews %lld\n", m_swapChain.getSwapChainImageviews().size());

        m_swapChain.createSwapChain();
        m_swapChain.createSwapChainImageViews();

        m_depthImage = Image(m_device.lDevice(),
                m_device.pDevice(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                m_device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        karhu::createFrameBuffer(m_device.lDevice(), m_framebuffers,
                m_swapChain.getSwapChainImageviews(),
                m_renderPasses[0].getRenderPass(),
                m_swapChain.getSwapChainExtent().width,
                m_swapChain.getSwapChainExtent().height,
                1,
                false,
                m_depthImage.getImageView());
    }
} // namespace karhu
