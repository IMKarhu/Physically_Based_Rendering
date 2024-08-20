#include "kApplication.hpp"
#include "kBuffer.hpp"
#include "kModel.hpp"




namespace karhu
{
    Application::Application()
    {
        m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);

    }

    Application::~Application()
    {
        printf("app destroyed\n");
        cleanUpSwapChain();
        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            vkDestroyBuffer(m_VkDevice->m_Device, m_UniformBuffers[i], nullptr);
            vkFreeMemory(m_VkDevice->m_Device, m_UniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(m_VkDevice->m_Device, m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_VkDevice->m_Device, m_DescriptorLayout, nullptr);
        vkDestroyBuffer(m_VkDevice->m_Device, m_IndexBuffer, nullptr);
        vkFreeMemory(m_VkDevice->m_Device, m_IndexBufferMemory, nullptr);
        vkDestroyBuffer(m_VkDevice->m_Device, m_VertexBuffer, nullptr);
        vkFreeMemory(m_VkDevice->m_Device, m_VertexBufferMemory, nullptr);
        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            vkDestroySemaphore(m_VkDevice->m_Device, m_Semaphores.availableSemaphores[i], nullptr);
            vkDestroySemaphore(m_VkDevice->m_Device, m_Semaphores.finishedSemaphores[i], nullptr);
            vkDestroyFence(m_VkDevice->m_Device, m_InFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(m_VkDevice->m_Device, m_CommandPool, nullptr);

        vkDestroyPipeline(m_VkDevice->m_Device, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_VkDevice->m_Device, m_PipelineLayout, nullptr);
        vkDestroyRenderPass(m_VkDevice->m_Device, m_RenderPass, nullptr);
        vkDestroySurfaceKHR(m_Window->getInstance(), m_Surface, nullptr);
        if (enableValidationLayers)
        {
            destroyDebugUtilsMessengerEXT(m_Window->getInstance(), m_DebugMessenger, nullptr);
        }
    }

    void Application::run()
    {
        setupDebugMessenger();
        createSurface();
        m_VkDevice = std::make_shared<Vulkan_Device>(m_Window->getInstance(), m_Surface);
        m_VkDevice->pickPhysicalDevice();
        m_VkDevice->createLogicalDevice();
        VkSwapchainCreateInfoKHR createinfo = fillSwapchainCI();
        printf("before swapchain creation.");
        m_VkSwapChain = std::make_shared<Vulkan_SwapChain>(m_VkDevice->m_Device, m_Window->getWindow());
        m_VkSwapChain->createSwapChain(m_VkDevice->querySwapChainSupport(m_VkDevice->m_PhysicalDevice), m_Surface, createinfo);
        m_VkSwapChain->createImageViews();
        createRenderPass();

        VkDescriptorSetLayoutBinding binding{};
        binding = Helpers::fillLayoutBindingStruct(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding = Helpers::fillLayoutBindingStruct(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { binding, samplerBinding };

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo = Helpers::fillDescriptorSetLayoutCreateInfo();
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_VkDevice->m_Device, &createInfo, nullptr, &m_DescriptorLayout));

        //m_Texture = std::make_shared<Texture>(m_VkDevice);
        m_Model = std::make_shared<vkglTFModel>(m_VkDevice);

        createGraphicsPipeline();
        createCommandPool();
        createDepthResources();
        createFrameBuffers();
        
        m_Model->m_Texture.createTexture(m_CommandPool);
        m_Model->m_Texture.createTextureImageView();
        m_Model->m_Texture.createSampler();
        /*createVertexBuffer();
        createIndexBuffer();*/
        loadGltfFile("../models/Cube.gltf");
        createUniformBuffers();

        /*Descriptor pool creation.*/
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_MaxFramesInFlight);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_MaxFramesInFlight);

        VkDescriptorPoolCreateInfo poolcreateInfo{};
        poolcreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolcreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolcreateInfo.pPoolSizes = poolSizes.data();
        poolcreateInfo.maxSets = static_cast<uint32_t>(m_MaxFramesInFlight);

        VK_CHECK(vkCreateDescriptorPool(m_VkDevice->m_Device, &poolcreateInfo, nullptr, &m_DescriptorPool));

        /*Descriptor set creation.*/
        std::vector<VkDescriptorSetLayout> layouts(m_MaxFramesInFlight, m_DescriptorLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_MaxFramesInFlight);
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(m_MaxFramesInFlight);
        VK_CHECK(vkAllocateDescriptorSets(m_VkDevice->m_Device, &allocInfo, m_DescriptorSets.data()));

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_Model->m_Texture.imageView();
            imageInfo.sampler = m_Model->m_Texture.textureSampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_VkDevice->m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        createCommandBuffers();
        createSyncObjects();


        update(m_DeltaTime);
    }

    void Application::createGraphicsPipeline()
    {
        auto vertexCode = readFile("../shaders/vertexShader.spv");
        auto fragmentCode = readFile("../shaders/fragmentShader.spv");

        VkShaderModule vertexShaderModule = createShaderModule(vertexCode);
        VkShaderModule fragmentShaderModule = createShaderModule(fragmentCode);

        VkPipelineShaderStageCreateInfo vertexShaderStageCI{};
        vertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageCI.module = vertexShaderModule;
        vertexShaderStageCI.pName = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderStageCI{};
        fragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageCI.module = fragmentShaderModule;
        fragmentShaderStageCI.pName = "main";

        VkPipelineShaderStageCreateInfo stages[] = { vertexShaderStageCI, fragmentShaderStageCI };

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescription = Vertex::getAttributeDescription();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_VkSwapChain->m_SwapChainExtent.width;
        viewport.height = (float)m_VkSwapChain->m_SwapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_VkSwapChain->m_SwapChainExtent;

        VkPipelineDynamicStateCreateInfo dynamiccreateinfo{};
        dynamiccreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamiccreateinfo.dynamicStateCount = static_cast<uint32_t>(m_DynamicStates.size());
        dynamiccreateinfo.pDynamicStates = m_DynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // nullptr if no descriptors used
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorLayout; // nullptr if no descriptors used
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        VK_CHECK(vkCreatePipelineLayout(m_VkDevice->m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamiccreateinfo;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = m_RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        VK_CHECK(vkCreateGraphicsPipelines(m_VkDevice->m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));

        vkDestroyShaderModule(m_VkDevice->m_Device, vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_VkDevice->m_Device, fragmentShaderModule, nullptr);
    }

    void Application::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_VkSwapChain->m_SwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;


        VK_CHECK(vkCreateRenderPass(m_VkDevice->m_Device, &renderPassInfo, nullptr, &m_RenderPass));

    }

    void Application::createFrameBuffers()
    {
        m_FrameBuffers.resize(m_VkSwapChain->m_SwapChainImageViews.size());

        for (size_t i = 0; i < m_VkSwapChain->m_SwapChainImageViews.size(); i++)
        {
            std::array<VkImageView, 2> attachments = {
                m_VkSwapChain->m_SwapChainImageViews[i],
                m_DepthImageView
            };


            VkFramebufferCreateInfo createinfo{};
            createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createinfo.renderPass = m_RenderPass;
            createinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createinfo.pAttachments = attachments.data();
            createinfo.width = m_VkSwapChain->m_SwapChainExtent.width;
            createinfo.height = m_VkSwapChain->m_SwapChainExtent.height;
            createinfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_VkDevice->m_Device, &createinfo, nullptr, &m_FrameBuffers[i]));
        }
    }

    void Application::createCommandPool()
    {
        QueueFamilyIndices indices = m_VkDevice->findQueueFamilies(m_VkDevice->m_PhysicalDevice);

        VkCommandPoolCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createinfo.queueFamilyIndex = indices.graphicsFamily.value();

        VK_CHECK(vkCreateCommandPool(m_VkDevice->m_Device, &createinfo, nullptr, &m_CommandPool));
    }

    void Application::createCommandBuffers()
    {
        m_CommandBuffers.resize(m_MaxFramesInFlight);

        VkCommandBufferAllocateInfo allocinfo{};
        allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocinfo.commandPool = m_CommandPool;
        allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocinfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

        VK_CHECK(vkAllocateCommandBuffers(m_VkDevice->m_Device, &allocinfo, m_CommandBuffers.data()));
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
        renderpassInfo.renderPass = m_RenderPass;
        renderpassInfo.framebuffer = m_FrameBuffers[index];
        renderpassInfo.renderArea.offset = { 0,0 };
        renderpassInfo.renderArea.extent = m_VkSwapChain->m_SwapChainExtent;

        std::array<VkClearValue, 2> clearColors{};
        clearColors[0].color = { {0.0f ,0.0f ,0.0f ,1.0f} };
        clearColors[1].depthStencil = { 1.0f, 0 };
        renderpassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
        renderpassInfo.pClearValues = clearColors.data();

        vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = static_cast<float>(m_VkSwapChain->m_SwapChainExtent.width);
        viewPort.height = static_cast<float>(m_VkSwapChain->m_SwapChainExtent.height);
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewPort);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_VkSwapChain->m_SwapChainExtent;

        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vBuffers[] = { m_Model->m_VertexBuffer.m_Buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_Model->m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[index], 0, nullptr);

        //VertexCount = 3, we techincally have 3 vertices to draw, instanceCount = 1, used for instance rendering, we use one because we dont have any instances
        //firstVertex = 0 offset into the vertex buffer, defines lowest value of gl_VertexIndex
        //firstInstance = 0 offset of instance, defines lowest value of gl_VertexIndex.
        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
        m_Model->draw(commandBuffer, m_PipelineLayout, m_DescriptorSets, index);
        //vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);

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
            if (vkCreateSemaphore(m_VkDevice->m_Device, &createinfo, nullptr, &m_Semaphores.availableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_VkDevice->m_Device, &createinfo, nullptr, &m_Semaphores.finishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_VkDevice->m_Device, &fenceinfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create Semaphores!");
            }
        }

    }

    void Application::createVertexBuffer()
    {
        /*Buffer buffer = { m_VkDevice };
        buffer.createVkBuffer<Vertex>(m_Vertices, m_VertexBuffer, m_VertexBufferMemory,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_CommandPool);*/
    }

    void Application::createIndexBuffer()
    {
        /*Buffer buffer = { m_VkDevice };
        buffer.createVkBuffer<uint32_t>(m_Indices, m_IndexBuffer, m_IndexBufferMemory,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_CommandPool);*/
    }

    void Application::createUniformBuffers()
    {
        Buffer buffer = { m_VkDevice };
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(m_MaxFramesInFlight);
        m_UniformBuffersMemory.resize(m_MaxFramesInFlight);
        m_UniformBuffersMapped.resize(m_MaxFramesInFlight);

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            buffer.createBuffers(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_UniformBuffers[i], m_UniformBuffersMemory[i]);
            vkMapMemory(m_VkDevice->m_Device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
        }
    }

    void Application::createDepthResources()
    {
        VkFormat format = findDepthFormat();

        m_Model->m_Texture.createImage(m_VkSwapChain->m_SwapChainExtent.width, m_VkSwapChain->m_SwapChainExtent.height, format, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
        m_DepthImageView = m_Model->m_Texture.createImageview(m_DepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);
        
    }

    void Application::updateUBOs(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_VkSwapChain->m_SwapChainExtent.width / (float)m_VkSwapChain->m_SwapChainExtent.height, 0.1f, 10.0f);
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
        vkDeviceWaitIdle(m_VkDevice->m_Device);
    }

    void Application::drawFrame()
    {
        vkWaitForFences(m_VkDevice->m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult res = vkAcquireNextImageKHR(m_VkDevice->m_Device, m_VkSwapChain->m_SwapChain, UINT64_MAX, m_Semaphores.availableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            reCreateSwapChain();
            return;
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        vkResetFences(m_VkDevice->m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

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

        VK_CHECK(vkQueueSubmit(m_VkDevice->m_GraphicsQueue, 1, &submitinfo, m_InFlightFences[m_CurrentFrame]));

        VkPresentInfoKHR presentinfo{};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_VkSwapChain->m_SwapChain };
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = swapChains;
        presentinfo.pImageIndices = &imageIndex;
        presentinfo.pResults = nullptr;

        res = vkQueuePresentKHR(m_VkDevice->m_PresentQueue, &presentinfo);

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

    void Application::createSurface()
    {
        VK_CHECK(glfwCreateWindowSurface(m_Window->getInstance(), m_Window->getWindow(), nullptr, &m_Surface));
    }

    VkSwapchainCreateInfoKHR Application::fillSwapchainCI()
    {
        VkSwapchainCreateInfoKHR createinfo{};
        QueueFamilyIndices indices = m_VkDevice->findQueueFamilies(m_VkDevice->m_PhysicalDevice);

        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createinfo.queueFamilyIndexCount = 2;
            createinfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createinfo.queueFamilyIndexCount = 0;
            createinfo.pQueueFamilyIndices = nullptr;
        }
        return createinfo;
    }

    std::vector<char> Application::readFile(const std::string& fileName)
    {
        //read  from the end of thefile and as binary file.
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file!\n");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    VkShaderModule Application::createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createinfo.codeSize = code.size();
        createinfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        VK_CHECK(vkCreateShaderModule(m_VkDevice->m_Device, &createinfo, nullptr, &shaderModule));

        return shaderModule;
    }

    void Application::cleanUpSwapChain()
    {
        vkDestroyImageView(m_VkDevice->m_Device, m_DepthImageView, nullptr);
        vkDestroyImage(m_VkDevice->m_Device, m_DepthImage, nullptr);
        vkFreeMemory(m_VkDevice->m_Device, m_DepthImageMemory, nullptr);
        for (auto framebuffer : m_FrameBuffers)
        {
            vkDestroyFramebuffer(m_VkDevice->m_Device, framebuffer, nullptr);
        }
        for (auto imageView : m_VkSwapChain->m_SwapChainImageViews)
        {
            vkDestroyImageView(m_VkDevice->m_Device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_VkDevice->m_Device, m_VkSwapChain->m_SwapChain, nullptr);
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

        VK_CHECK(vkDeviceWaitIdle(m_VkDevice->m_Device));

        cleanUpSwapChain();

        VkSwapchainCreateInfoKHR createinfo = fillSwapchainCI();
        m_VkSwapChain->createSwapChain(m_VkDevice->querySwapChainSupport(m_VkDevice->m_PhysicalDevice), m_Surface, createinfo);
        m_VkSwapChain->createImageViews();
        createDepthResources();
        createFrameBuffers();
    }

    VkFormat Application::findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : formats)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_VkDevice->m_PhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat Application::findDepthFormat()
    {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool Application::hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void Application::loadGltfFile(std::string fileName)
    {
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        m_Model->loadgltfFile(fileName, m_Indices, m_Vertices);

        

        Buffer buffer = { m_VkDevice };
        buffer.createVkBuffer<Vertex>(m_Vertices, m_Model->m_VertexBuffer.m_Buffer, m_Model->m_VertexBuffer.m_BufferMemory,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_CommandPool);

        buffer.createVkBuffer<uint32_t>(m_Indices, m_Model->m_IndexBuffer.m_Buffer, m_Model->m_IndexBuffer.m_BufferMemory,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_CommandPool);


    }



} // namespace karhu
