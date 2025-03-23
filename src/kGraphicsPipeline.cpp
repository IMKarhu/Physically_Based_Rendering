#include "kGraphicsPipeline.hpp"
#include "utils/macros.hpp"
#include "types.hpp"
#include <fstream>


namespace karhu
{
	kGraphicsPipeline::kGraphicsPipeline(Vulkan_Device& device)
		:m_Device(device)
	{
	}

	kGraphicsPipeline::~kGraphicsPipeline()
	{
        vkDestroyPipeline(m_Device.m_Device, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_Device.m_Device, m_PipelineLayout, nullptr);
        vkDestroyRenderPass(m_Device.m_Device, m_RenderPass, nullptr);
	}

	void kGraphicsPipeline::createPipeline(GraphicsPipelineStruct pipelineStruct, const std::string& vertfilePath, const std::string& fragfilePath)
	{
        auto vertexCode = readFile(vertfilePath);
        auto fragmentCode = readFile(fragfilePath);

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

        pipelineStruct.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineStruct.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineStruct.inputAssembly.primitiveRestartEnable = VK_FALSE;

        pipelineStruct.viewport.x = 0.0f;
        pipelineStruct.viewport.y = 0.0f;
        //pipelineStruct.viewport.width = (float)m_VkSwapChain.m_SwapChainExtent.width; SET IN APPLICATION.CPP
        //pipelineStruct.viewport.height = (float)m_VkSwapChain.m_SwapChainExtent.height; SET IN APPLICATION.CPP
        pipelineStruct.viewport.minDepth = 0.0f;
        pipelineStruct.viewport.maxDepth = 1.0f;

        pipelineStruct.scissor.offset = { 0, 0 };
       // pipelineStruct.scissor.extent = m_VkSwapChain.m_SwapChainExtent; SET IN APPLICATION.CPP

        pipelineStruct.dynamiccreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineStruct.dynamiccreateinfo.dynamicStateCount = static_cast<uint32_t>(m_DynamicStates.size());
        pipelineStruct.dynamiccreateinfo.pDynamicStates = m_DynamicStates.data();

        pipelineStruct.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineStruct.viewportState.viewportCount = 1;
        pipelineStruct.viewportState.pViewports = &pipelineStruct.viewport;
        pipelineStruct.viewportState.scissorCount = 1;
        pipelineStruct.viewportState.pScissors = &pipelineStruct.scissor;

        pipelineStruct.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineStruct.rasterizer.depthClampEnable = VK_FALSE;
        pipelineStruct.rasterizer.rasterizerDiscardEnable = VK_FALSE;
        pipelineStruct.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        pipelineStruct.rasterizer.lineWidth = 1.0f;
        pipelineStruct.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineStruct.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pipelineStruct.rasterizer.depthBiasEnable = VK_FALSE;
        pipelineStruct.rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        pipelineStruct.rasterizer.depthBiasClamp = 0.0f; // Optional
        pipelineStruct.rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        pipelineStruct.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineStruct.multisampling.sampleShadingEnable = VK_FALSE;
        pipelineStruct.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineStruct.multisampling.minSampleShading = 1.0f; // Optional
        pipelineStruct.multisampling.pSampleMask = nullptr; // Optional
        pipelineStruct.multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        pipelineStruct.multisampling.alphaToOneEnable = VK_FALSE; // Optional

        pipelineStruct.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipelineStruct.colorBlendAttachment.blendEnable = VK_TRUE;
        pipelineStruct.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineStruct.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineStruct.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        pipelineStruct.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipelineStruct.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipelineStruct.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        pipelineStruct.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineStruct.depthStencil.depthTestEnable = VK_TRUE;
        pipelineStruct.depthStencil.depthWriteEnable = VK_TRUE;
        pipelineStruct.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        pipelineStruct.depthStencil.depthBoundsTestEnable = VK_FALSE;
        pipelineStruct.depthStencil.minDepthBounds = 0.0f; // Optional
        pipelineStruct.depthStencil.maxDepthBounds = 1.0f; // Optional
        pipelineStruct.depthStencil.stencilTestEnable = VK_FALSE;
        pipelineStruct.depthStencil.front = {}; // Optional
        pipelineStruct.depthStencil.back = {}; // Optional

        pipelineStruct.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineStruct.colorBlending.logicOpEnable = VK_FALSE;
        pipelineStruct.colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        pipelineStruct.colorBlending.attachmentCount = 1;
        pipelineStruct.colorBlending.pAttachments = &pipelineStruct.colorBlendAttachment;
        pipelineStruct.colorBlending.blendConstants[0] = 0.0f; // Optional
        pipelineStruct.colorBlending.blendConstants[1] = 0.0f; // Optional
        pipelineStruct.colorBlending.blendConstants[2] = 0.0f; // Optional
        pipelineStruct.colorBlending.blendConstants[3] = 0.0f; // Optional

        pipelineStruct.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        
        //pipelineStruct.pipelineLayoutInfo.pSetLayouts = &m_DescriptorLayout; // nullptr if no descriptors used SET IN APPLICATION.CPP

        VkPushConstantRange objPushConstant{};
        objPushConstant.offset = 0;
        objPushConstant.size = sizeof(ObjPushConstant);
        objPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPushConstantRange cameraPushConstant{};
        cameraPushConstant.offset = 64;
        cameraPushConstant.size = sizeof(pushConstants);
        cameraPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkPushConstantRange> pushConstantRanges{
            objPushConstant,
            cameraPushConstant
        };

        pipelineStruct.pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()); // Optional
        pipelineStruct.pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data(); // Optional

        VK_CHECK(vkCreatePipelineLayout(m_Device.m_Device, &pipelineStruct.pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &pipelineStruct.inputAssembly;
        pipelineInfo.pViewportState = &pipelineStruct.viewportState;
        pipelineInfo.pRasterizationState = &pipelineStruct.rasterizer;
        pipelineInfo.pMultisampleState = &pipelineStruct.multisampling;
        pipelineInfo.pDepthStencilState = &pipelineStruct.depthStencil;
        pipelineInfo.pColorBlendState = &pipelineStruct.colorBlending;
        pipelineInfo.pDynamicState = &pipelineStruct.dynamiccreateinfo;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = m_RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        VK_CHECK(vkCreateGraphicsPipelines(m_Device.m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));

        vkDestroyShaderModule(m_Device.m_Device, vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_Device.m_Device, fragmentShaderModule, nullptr);
	}

	void kGraphicsPipeline::createRenderpass(VkFormat& swapchainImageFormat, VkFormat depthFormat)
	{
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
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


        VK_CHECK(vkCreateRenderPass(m_Device.m_Device, &renderPassInfo, nullptr, &m_RenderPass));
	}

    void kGraphicsPipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
    }

    std::vector<char> kGraphicsPipeline::readFile(const std::string& fileName)
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

    VkShaderModule kGraphicsPipeline::createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createinfo.codeSize = code.size();
        createinfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        VK_CHECK(vkCreateShaderModule(m_Device.m_Device, &createinfo, nullptr, &shaderModule));

        return shaderModule;
    }
}


