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

	void kGraphicsPipeline::createPipeline(GraphicsPipelineStruct pipelineStruct)
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
        pipelineStruct.pipelineLayoutInfo.setLayoutCount = 1; // nullptr if no descriptors used
        //pipelineStruct.pipelineLayoutInfo.pSetLayouts = &m_DescriptorLayout; // nullptr if no descriptors used SET IN APPLICATION.CPP
        pipelineStruct.pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineStruct.pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        VK_CHECK(vkCreatePipelineLayout(m_Device.m_Device, &pipelineStruct.pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        pipelineStruct.pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineStruct.pipelineInfo.stageCount = 2;
        pipelineStruct.pipelineInfo.pStages = stages;
        pipelineStruct.pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineStruct.pipelineInfo.pInputAssemblyState = &pipelineStruct.inputAssembly;
        pipelineStruct.pipelineInfo.pViewportState = &pipelineStruct.viewportState;
        pipelineStruct.pipelineInfo.pRasterizationState = &pipelineStruct.rasterizer;
        pipelineStruct.pipelineInfo.pMultisampleState = &pipelineStruct.multisampling;
        pipelineStruct.pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineStruct.pipelineInfo.pColorBlendState = &pipelineStruct.colorBlending;
        pipelineStruct.pipelineInfo.pDynamicState = &pipelineStruct.dynamiccreateinfo;
        pipelineStruct.pipelineInfo.layout = m_PipelineLayout;
        pipelineStruct.pipelineInfo.renderPass = m_RenderPass;
        pipelineStruct.pipelineInfo.subpass = 0;
        pipelineStruct.pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineStruct.pipelineInfo.basePipelineIndex = -1; // Optional

        VK_CHECK(vkCreateGraphicsPipelines(m_Device.m_Device, VK_NULL_HANDLE, 1, &pipelineStruct.pipelineInfo, nullptr, &m_GraphicsPipeline));

        vkDestroyShaderModule(m_Device.m_Device, vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_Device.m_Device, fragmentShaderModule, nullptr);
	}

	void kGraphicsPipeline::createRenderpass(VkFormat& swapchainImageFormat)
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

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;


        VK_CHECK(vkCreateRenderPass(m_Device.m_Device, &renderPassInfo, nullptr, &m_RenderPass));
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


