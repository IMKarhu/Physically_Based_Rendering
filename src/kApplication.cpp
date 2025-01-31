#include "kApplication.hpp"


namespace karhu
{
    Application::Application()
    {

    }

    Application::~Application()
    {
        printf("app destroyed\n");
        cleanUpSwapChain();
        for (size_t i = 0; i < m_VkSwapChain.m_MaxFramesInFlight; i++)
        {
            vkDestroyBuffer(m_VkDevice.m_Device, m_UniformBuffers[i], nullptr);
            vkFreeMemory(m_VkDevice.m_Device, m_UniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(m_VkDevice.m_Device, m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_VkDevice.m_Device, m_DescriptorLayout, nullptr);
        for (size_t i = 0; i < m_VkSwapChain.m_MaxFramesInFlight; i++)
        {
            vkDestroySemaphore(m_VkDevice.m_Device, m_Semaphores.availableSemaphores[i], nullptr);
            vkDestroySemaphore(m_VkDevice.m_Device, m_Semaphores.finishedSemaphores[i], nullptr);
            vkDestroyFence(m_VkDevice.m_Device, m_InFlightFences[i], nullptr);
        }
        vkDestroySurfaceKHR(m_Window->getInstance(), m_Window->getSurface(), nullptr);
    }

    void Application::run()
    {
        printf("before swapchain creation.");
        m_VkSwapChain.createSwapChain(m_Window->getSurface(), m_Window->getWindow());
        m_VkSwapChain.createImageViews();
        m_GraphicsPipeline.createRenderpass(m_VkSwapChain.m_SwapChainImageFormat, findDepthFormat());

        VkDescriptorSetLayoutBinding binding{};
        binding = Helpers::fillLayoutBindingStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo = Helpers::fillDescriptorSetLayoutCreateInfo();
        createInfo.bindingCount = 1;
        createInfo.pBindings = &binding;

        VK_CHECK(vkCreateDescriptorSetLayout(m_VkDevice.m_Device, &createInfo, nullptr, &m_DescriptorLayout));

        createGraphicsPipeline();
        createDepthResources();
        createFrameBuffers();
        m_Model = std::make_unique<kModel>(m_VkDevice, m_Vertices, m_Indices, m_VkSwapChain.m_CommandPool);
        createUniformBuffers();

        /*Descriptor pool creation.*/
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(m_VkSwapChain.m_MaxFramesInFlight);

        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = 1;
        poolcreateInfo.pPoolSizes = &poolSize;
        poolcreateInfo.maxSets = static_cast<uint32_t>(m_VkSwapChain.m_MaxFramesInFlight);

        VK_CHECK(vkCreateDescriptorPool(m_VkDevice.m_Device, &poolcreateInfo, nullptr, &m_DescriptorPool));

        /*Descriptor set creation.*/
        std::vector<VkDescriptorSetLayout> layouts(m_VkSwapChain.m_MaxFramesInFlight, m_DescriptorLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_VkSwapChain.m_MaxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();
        printf("hello3\n");
        m_DescriptorSets.resize(m_VkSwapChain.m_MaxFramesInFlight);
        VK_CHECK(vkAllocateDescriptorSets(m_VkDevice.m_Device, &allocInfo, m_DescriptorSets.data()));

        for (size_t i = 0; i < m_VkSwapChain.m_MaxFramesInFlight; i++)
        {
            printf("hello: %d\n", i);
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_DescriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(m_VkDevice.m_Device, 1, &descriptorWrite, 0, nullptr);
        }

        createSyncObjects();


        update(m_DeltaTime);
    }

 
    void Application::createFrameBuffers()
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
            createinfo.renderPass = m_GraphicsPipeline.getRenderPass();
            createinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createinfo.pAttachments = attachments.data();
            createinfo.width = m_VkSwapChain.m_SwapChainExtent.width;
            createinfo.height = m_VkSwapChain.m_SwapChainExtent.height;
            createinfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_VkDevice.m_Device, &createinfo, nullptr, &m_FrameBuffers[i]));
        }
    }

    void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index)
    {
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = 0;
        info.pInheritanceInfo = nullptr;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &info));

        VkRenderPassBeginInfo renderpassInfo{};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassInfo.renderPass = m_GraphicsPipeline.getRenderPass();
        renderpassInfo.framebuffer = m_FrameBuffers[index];
        renderpassInfo.renderArea.offset = { 0,0 };
        renderpassInfo.renderArea.extent = m_VkSwapChain.m_SwapChainExtent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = { {0.0f ,0.0f ,0.0f ,1.0f} };
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

        /*VkBuffer vBuffers[] = {m_VertexBuffer};
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);*/
        m_Model->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.getPipelineLayout(), 0, 1, &m_DescriptorSets[index], 0, nullptr);

        //VertexCount = 3, we techincally have 3 vertices to draw, instanceCount = 1, used for instance rendering, we use one because we dont have any instances
        //firstVertex = 0 offset into the vertex buffer, defines lowest value of gl_VertexIndex
        //firstInstance = 0 offset of instance, defines lowest value of gl_VertexIndex.
        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
        //vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
        m_Model->draw(commandBuffer, m_Indices);

        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void Application::createSyncObjects()
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

    void Application::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(m_VkSwapChain.m_MaxFramesInFlight);
        m_UniformBuffersMemory.resize(m_VkSwapChain.m_MaxFramesInFlight);
        m_UniformBuffersMapped.resize(m_VkSwapChain.m_MaxFramesInFlight);

        for (size_t i = 0; i < m_VkSwapChain.m_MaxFramesInFlight; i++)
        {
            m_VkDevice.createBuffers(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_UniformBuffers[i], m_UniformBuffersMemory[i]);
            vkMapMemory(m_VkDevice.m_Device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
        }
    }

    void Application::updateUBOs(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_VkSwapChain.m_SwapChainExtent.width / (float)m_VkSwapChain.m_SwapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void Application::update(float deltaTime)
    {
        while (!m_Window->shouldClose())
        {
            m_Window->pollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(m_VkDevice.m_Device);
    }

    void Application::drawFrame()
    {
        vkWaitForFences(m_VkDevice.m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult res = vkAcquireNextImageKHR(m_VkDevice.m_Device, m_VkSwapChain.m_SwapChain, UINT64_MAX, m_Semaphores.availableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            reCreateSwapChain();
            return;
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        vkResetFences(m_VkDevice.m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(m_VkSwapChain.m_CommandBuffers[m_CurrentFrame], 0);
        recordCommandBuffer(m_VkSwapChain.m_CommandBuffers[m_CurrentFrame], imageIndex);

        updateUBOs(imageIndex);

        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_Semaphores.availableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &m_VkSwapChain.m_CommandBuffers[m_CurrentFrame];
        VkSemaphore signalSemaphores[] = { m_Semaphores.finishedSemaphores[m_CurrentFrame] };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkQueueSubmit(m_VkDevice.m_GraphicsQueue, 1, &submitinfo, m_InFlightFences[m_CurrentFrame]));

        VkPresentInfoKHR presentinfo{};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_VkSwapChain.m_SwapChain };
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = swapChains;
        presentinfo.pImageIndices = &imageIndex;
        presentinfo.pResults = nullptr;

        res = vkQueuePresentKHR(m_VkDevice.m_PresentQueue, &presentinfo);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR || m_Window->getResize())
        {
            m_Window->setResize(false);
            reCreateSwapChain();
        }
        else if (res != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swapchain image1\n");
        }
        m_CurrentFrame = (m_CurrentFrame + 1) % m_VkSwapChain.m_MaxFramesInFlight;
    }
    
    void Application::createGraphicsPipeline()
    {
        GraphicsPipelineStruct pipelineStruct{};
        pipelineStruct.viewportWidth = m_VkSwapChain.m_SwapChainExtent.width;
        pipelineStruct.viewportheight = m_VkSwapChain.m_SwapChainExtent.height;
        pipelineStruct.scissor.extent = m_VkSwapChain.m_SwapChainExtent;
        pipelineStruct.pipelineLayoutInfo.pSetLayouts = &m_DescriptorLayout;

        m_GraphicsPipeline.createPipeline(pipelineStruct);
    }

    void Application::createDepthResources()
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

    VkFormat Application::findDepthFormat()
    {
        return m_VkDevice.findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool Application::hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void Application::createImage(uint32_t width,
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

    void Application::cleanUpSwapChain()
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

    void Application::reCreateSwapChain()
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



} // namespace karhu
