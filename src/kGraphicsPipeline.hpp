#pragma once
#include <vulkan/vulkan.h>
#include "kDevice.hpp"

#include <vector>
#include <string>


namespace karhu
{

	struct GraphicsPipelineCreateStruct
	{
		VkViewport viewport{};
		float viewportWidth;
		float viewportheight;
		VkRect2D scissor{};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		VkPipelineDynamicStateCreateInfo dynamiccreateinfo{};
		VkPipelineViewportStateCreateInfo viewportState{};
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		VkPipelineMultisampleStateCreateInfo multisampling{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		VkGraphicsPipelineCreateInfo pipelineInfo{};
	};

	class GraphicsPipeline
	{
	public:
		GraphicsPipeline(Vulkan_Device& device);
		~GraphicsPipeline();

		GraphicsPipeline(const GraphicsPipeline&) = delete;
		void operator=(const GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

		void createGraphicsPipeline(GraphicsPipelineCreateStruct pipelineStruct);
		void createRenderPass(VkFormat& swapChainImageFormat);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();

		VkRenderPass getRenderPass() const { return m_RenderPass; }
		VkPipeline getPipeline() const { return m_GraphicsPipeline; }
		VkPipelineLayout getLayout() const { return m_PipelineLayout; }
		
	private:
		static std::vector<char> readFile(const std::string& fileName);
		VkShaderModule createShaderModule(const std::vector<char>& code);
	private:
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkRenderPass m_RenderPass;

		std::vector<VkDynamicState> m_DynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		Vulkan_Device& m_VkDevice;
	};
}