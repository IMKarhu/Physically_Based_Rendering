#include "vkPipeLine.h"
#include "vkDevice.h"
#include "model.h"

#include <iostream>
#include <fstream>

namespace kge
{
	vkPipeline::vkPipeline(vkDevice* device, const std::string& vertfilepath, const std::string& fragfilepath, const vkPipelineConfigInfo& configInfo)
		:m_Device(device)
	{
		createPipeLine(vertfilepath, fragfilepath, configInfo);
	}

	vkPipeline::~vkPipeline()
	{
		std::cout << "Pipeline Destructor called!\n" << std::endl;
		vkDestroyPipeline(m_Device->getDevice(), m_Pipeline, nullptr);
	}

	void vkPipeline::bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}

	vkPipelineConfigInfo& vkPipeline::createPipeLineConfigInfo(vkPipelineConfigInfo& configInfo,uint32_t width, uint32_t height)
	{
		/*vkPipelineConfigInfo configInfo{};*/

		configInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

		configInfo.viewport.x = 0.0f;
		configInfo.viewport.y = 0.0f;
		configInfo.viewport.width = static_cast<float>(width);
		configInfo.viewport.height = static_cast<float>(height);
		configInfo.viewport.minDepth = 0.0f;
		configInfo.viewport.maxDepth = 1.0f;

		configInfo.scissor.offset = { 0, 0 };
		configInfo.scissor.extent = { width,height };

		configInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizer.depthClampEnable = VK_FALSE;
		configInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizer.lineWidth = 1.0f;
		configInfo.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		configInfo.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configInfo.rasterizer.depthBiasEnable = VK_FALSE;
		configInfo.rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		configInfo.rasterizer.depthBiasClamp = 0.0f; // Optional
		configInfo.rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		configInfo.multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multiSampling.sampleShadingEnable = VK_FALSE;
		configInfo.multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multiSampling.minSampleShading = 1.0f; // Optional
		configInfo.multiSampling.pSampleMask = nullptr; // Optional
		configInfo.multiSampling.alphaToCoverageEnable = VK_FALSE; // Optional
		configInfo.multiSampling.alphaToOneEnable = VK_FALSE; // Optional

		configInfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencil.depthTestEnable = VK_TRUE;
		configInfo.depthStencil.depthWriteEnable = VK_TRUE;
		configInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencil.stencilTestEnable = VK_FALSE;

		configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		configInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlending.logicOpEnable = VK_FALSE;
		configInfo.colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		configInfo.colorBlending.attachmentCount = 1;
		configInfo.colorBlending.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlending.blendConstants[0] = 0.0f; // Optional
		configInfo.colorBlending.blendConstants[1] = 0.0f; // Optional
		configInfo.colorBlending.blendConstants[2] = 0.0f; // Optional
		configInfo.colorBlending.blendConstants[3] = 0.0f; // Optional

		return configInfo;
	}

	void vkPipeline::createPipeLine(const std::string& vertfilepath, const std::string& fragfilepath, const vkPipelineConfigInfo& configInfo)
	{
		auto vertShaderCode = readFile(vertfilepath);
		auto fragShaderCode = readFile(fragfilepath);

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		// structure to hold vertexshader info
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		// structure to hold fragment shader info
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindigDescriptions = kgeModel::Vertex::getbindingDescriptions();
		auto attributeDescriptions = kgeModel::Vertex::getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindigDescriptions;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &configInfo.viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &configInfo.scissor;

		std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &configInfo.rasterizer;
		pipelineInfo.pMultisampleState = &configInfo.multiSampling;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencil;
		pipelineInfo.pColorBlendState = &configInfo.colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = configInfo.pipeLineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	

		if (vkCreateGraphicsPipelines(m_Device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(m_Device->getDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(m_Device->getDevice(), vertShaderModule, nullptr);
	}

	VkShaderModule vkPipeline::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_Device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	std::vector<char> vkPipeline::readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
}