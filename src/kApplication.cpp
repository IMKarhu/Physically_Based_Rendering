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
        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            vkDestroyBuffer(m_VkDevice.m_Device, m_UniformBuffers[i], nullptr);
            vkFreeMemory(m_VkDevice.m_Device, m_UniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(m_VkDevice.m_Device, m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_VkDevice.m_Device, m_DescriptorLayout, nullptr);
        vkDestroyBuffer(m_VkDevice.m_Device, m_IndexBuffer, nullptr);
        vkFreeMemory(m_VkDevice.m_Device, m_IndexBufferMemory, nullptr);
        vkDestroyBuffer(m_VkDevice.m_Device, m_VertexBuffer, nullptr);
        vkFreeMemory(m_VkDevice.m_Device, m_VertexBufferMemory, nullptr);
        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            vkDestroySemaphore(m_VkDevice.m_Device, m_Semaphores.availableSemaphores[i], nullptr);
            vkDestroySemaphore(m_VkDevice.m_Device, m_Semaphores.finishedSemaphores[i], nullptr);
            vkDestroyFence(m_VkDevice.m_Device, m_InFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(m_VkDevice.m_Device, m_CommandPool, nullptr);

        //vkDestroyPipeline(m_VkDevice.m_Device, m_GraphicsPipeline, nullptr);
        //vkDestroyPipelineLayout(m_VkDevice.m_Device, m_PipelineLayout, nullptr);
        //vkDestroyRenderPass(m_VkDevice.m_Device, m_RenderPass, nullptr);
        vkDestroySurfaceKHR(m_Window->getInstance(), m_Window->getSurface(), nullptr);
        if (enableValidationLayers)
        {
            destroyDebugUtilsMessengerEXT(m_Window->getInstance(), m_DebugMessenger, nullptr);
        }
    }

    void Application::run()
    {
        setupDebugMessenger();
        m_VkDevice.init();
        printf("before swapchain creation.");
        m_VkSwapChain.createSwapChain(m_Window->getSurface(), m_Window->getWindow());
        m_VkSwapChain.createImageViews();
        m_GraphicsPipeline.createRenderpass(m_VkSwapChain.m_SwapChainImageFormat);

        VkDescriptorSetLayoutBinding binding{};
        binding = Helpers::fillLayoutBindingStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo = Helpers::fillDescriptorSetLayoutCreateInfo();
        createInfo.bindingCount = 1;
        createInfo.pBindings = &binding;

        VK_CHECK(vkCreateDescriptorSetLayout(m_VkDevice.m_Device, &createInfo, nullptr, &m_DescriptorLayout));

        createGraphicsPipeline();
        createFrameBuffers();
        createCommandPool();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();

        /*Descriptor pool creation.*/
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(m_MaxFramesInFlight);

        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = 1;
        poolcreateInfo.pPoolSizes = &poolSize;
        poolcreateInfo.maxSets = static_cast<uint32_t>(m_MaxFramesInFlight);

        VK_CHECK(vkCreateDescriptorPool(m_VkDevice.m_Device, &poolcreateInfo, nullptr, &m_DescriptorPool));

        /*Descriptor set creation.*/
        std::vector<VkDescriptorSetLayout> layouts(m_MaxFramesInFlight, m_DescriptorLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_MaxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();
        printf("hello3\n");
        m_DescriptorSets.resize(m_MaxFramesInFlight);
        VK_CHECK(vkAllocateDescriptorSets(m_VkDevice.m_Device, &allocInfo, m_DescriptorSets.data()));

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
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

        createCommandBuffers();
        createSyncObjects();


        update(m_DeltaTime);
    }

 
    void Application::createFrameBuffers()
    {
        m_FrameBuffers.resize(m_VkSwapChain.m_SwapChainImageViews.size());

        for (size_t i = 0; i < m_VkSwapChain.m_SwapChainImageViews.size(); i++)
        {
            VkImageView attachments[]{
                m_VkSwapChain.m_SwapChainImageViews[i]
            };


            VkFramebufferCreateInfo createinfo{};
            createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createinfo.renderPass = m_GraphicsPipeline.getRenderPass();
            createinfo.attachmentCount = 1;
            createinfo.pAttachments = attachments;
            createinfo.width = m_VkSwapChain.m_SwapChainExtent.width;
            createinfo.height = m_VkSwapChain.m_SwapChainExtent.height;
            createinfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_VkDevice.m_Device, &createinfo, nullptr, &m_FrameBuffers[i]));
        }
    }

    void Application::createCommandPool()
    {
        QueueFamilyIndices indices = m_VkDevice.findQueueFamilies(m_VkDevice.m_PhysicalDevice);

        VkCommandPoolCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createinfo.queueFamilyIndex = indices.graphicsFamily.value();

        VK_CHECK(vkCreateCommandPool(m_VkDevice.m_Device, &createinfo, nullptr, &m_CommandPool));
    }

    void Application::createCommandBuffers()
    {
        m_CommandBuffers.resize(m_MaxFramesInFlight);

        VkCommandBufferAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocinfo.commandPool = m_CommandPool;
        allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocinfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

        VK_CHECK(vkAllocateCommandBuffers(m_VkDevice.m_Device, &allocinfo, m_CommandBuffers.data()));
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

        VkClearValue clearColor{ {{0.0f ,0.0f ,0.0f ,1.0f}} };
        renderpassInfo.clearValueCount = 1;
        renderpassInfo.pClearValues = &clearColor;

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

        VkBuffer vBuffers[] = { m_VertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.getPipelineLayout(), 0, 1, &m_DescriptorSets[index], 0, nullptr);

        //VertexCount = 3, we techincally have 3 vertices to draw, instanceCount = 1, used for instance rendering, we use one because we dont have any instances
        //firstVertex = 0 offset into the vertex buffer, defines lowest value of gl_VertexIndex
        //firstInstance = 0 offset of instance, defines lowest value of gl_VertexIndex.
        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void Application::createSyncObjects()
    {
        m_Semaphores.availableSemaphores.resize(m_MaxFramesInFlight);
        m_Semaphores.finishedSemaphores.resize(m_MaxFramesInFlight);
        m_InFlightFences.resize(m_MaxFramesInFlight);

        VkSemaphoreCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceinfo{};
        fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            if (vkCreateSemaphore(m_VkDevice.m_Device, &createinfo, nullptr, &m_Semaphores.availableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_VkDevice.m_Device, &createinfo, nullptr, &m_Semaphores.finishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_VkDevice.m_Device, &fenceinfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create Semaphores!");
            }
        }

    }

    void Application::createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        VK_CHECK(vkMapMemory(m_VkDevice.m_Device, stagingBufferMemory, 0, bufferSize, 0, &data));
        memcpy(data, m_Vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_VkDevice.m_Device, stagingBufferMemory);

        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffer, m_VertexBufferMemory);

        copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

        vkDestroyBuffer(m_VkDevice.m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_VkDevice.m_Device, stagingBufferMemory, nullptr);

    }

    void Application::createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        VK_CHECK(vkMapMemory(m_VkDevice.m_Device, stagingBufferMemory, 0, bufferSize, 0, &data));
        memcpy(data, m_Indices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_VkDevice.m_Device, stagingBufferMemory);

        createBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBuffer, m_IndexBufferMemory);

        copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

        vkDestroyBuffer(m_VkDevice.m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_VkDevice.m_Device, stagingBufferMemory, nullptr);
    }

    void Application::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(m_MaxFramesInFlight);
        m_UniformBuffersMemory.resize(m_MaxFramesInFlight);
        m_UniformBuffersMapped.resize(m_MaxFramesInFlight);

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            createBuffers(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_UniformBuffers[i], m_UniformBuffersMemory[i]);
            vkMapMemory(m_VkDevice.m_Device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
        }
    }

    void Application::createBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createinfo.size = size; //byte size of one vertices multiplied by size of vector
        createinfo.usage = usage;
        createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(m_VkDevice.m_Device, &createinfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_VkDevice.m_Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocinfo.allocationSize = memRequirements.size;
        /*VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/
        allocinfo.memoryTypeIndex = m_VkDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

        VK_CHECK(vkAllocateMemory(m_VkDevice.m_Device, &allocinfo, nullptr, &bufferMemory));
        //  printf("Hello\n");
        VK_CHECK(vkBindBufferMemory(m_VkDevice.m_Device, buffer, bufferMemory, 0));
    }

    void Application::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VK_CHECK(vkAllocateCommandBuffers(m_VkDevice.m_Device, &allocInfo, &commandBuffer));

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VK_CHECK(vkQueueSubmit(m_VkDevice.m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(m_VkDevice.m_GraphicsQueue));
        vkFreeCommandBuffers(m_VkDevice.m_Device, m_CommandPool, 1, &commandBuffer);
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

        vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
        recordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

        updateUBOs(imageIndex);

        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_Semaphores.availableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
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
        m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
    }

    void Application::setupDebugMessenger()
    {
        if (!enableValidationLayers)
        {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createinfo{};
        vkUtils::populateDebugMessengerCreateInfo(createinfo);

        if (createDebugUtilsMessengerEXT(m_Window->getInstance(), &createinfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create debug messenger!\n");
        }
    }

    VkResult Application::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void Application::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
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

    void Application::cleanUpSwapChain()
    {
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
        createFrameBuffers();
    }



} // namespace karhu
