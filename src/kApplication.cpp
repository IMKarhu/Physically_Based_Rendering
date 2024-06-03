#include "kApplication.hpp"


namespace karhu
{
    Application::Application()
    {
        m_Window = std::make_unique<kWindow>("Vulkan", 1080, 720);

    }

    Application::~Application()
    {

        printf("app destroyed\n");
        for (auto framebuffer : m_FrameBuffers)
        {
            vkDestroyFramebuffer(m_VkDevice->m_Device, framebuffer, nullptr);
        }
        vkDestroyPipeline(m_VkDevice->m_Device, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_VkDevice->m_Device, m_PipelineLayout, nullptr);
        vkDestroyRenderPass(m_VkDevice->m_Device, m_RenderPass, nullptr);
        vkDestroySwapchainKHR(m_VkDevice->m_Device, m_VkSwapChain->m_SwapChain, nullptr);
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
        //SwapChainSupportDetails details = m_VkDevice.querySwapChainSupport(m_VkDevice.m_PhysicalDevice);
        printf("before swapchain creation.");
        m_VkSwapChain = std::make_shared<Vulkan_SwapChain>(m_VkDevice->m_Device, m_Window->getWindow());
        m_VkSwapChain->createSwapChain(m_VkDevice->querySwapChainSupport(m_VkDevice->m_PhysicalDevice), m_Surface, createinfo);
        m_VkSwapChain->createImageViews();
        createRenderPass();
        createGraphicsPipeline();

        while (!m_Window->shouldClose())
        {
            m_Window->pollEvents();
        }
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

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

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
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
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
        pipelineInfo.pDepthStencilState = nullptr; // Optional
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

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        VK_CHECK(vkCreateRenderPass(m_VkDevice->m_Device, &renderPassInfo, nullptr, &m_RenderPass));

    }

    void Application::createFrameBuffers()
    {
        m_FrameBuffers.resize(m_VkSwapChain->m_SwapChainImageViews.size());

        for (size_t i = 0; i < m_VkSwapChain->m_SwapChainImageViews.size(); i++)
        {
            VkImageView attachments[]{
                m_VkSwapChain->m_SwapChainImageViews[i]
            };


            VkFramebufferCreateInfo createinfo{};
            createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createinfo.renderPass = m_RenderPass;
            createinfo.attachmentCount = 1;
            createinfo.pAttachments = attachments;
            createinfo.width = m_VkSwapChain->m_SwapChainExtent.width;
            createinfo.height = m_VkSwapChain->m_SwapChainExtent.height;
            createinfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_VkDevice->m_Device, &createinfo, nullptr, &m_FrameBuffers[i]));
        }
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



} // namespace karhu
