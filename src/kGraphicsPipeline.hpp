#pragma once
#include "vulkan/vulkan.h"
#include "kDevice.hpp"
#include <vector>

namespace karhu {
	struct GraphicsPipelineStruct
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
		VkRenderPass renderPass;
	};

	class kGraphicsPipeline
	{
	public:
		kGraphicsPipeline(Vulkan_Device& device);
		~kGraphicsPipeline();

		kGraphicsPipeline(const kGraphicsPipeline&) = delete;
		void operator=(const kGraphicsPipeline&) = delete;
		kGraphicsPipeline(kGraphicsPipeline&&) = delete;
		kGraphicsPipeline& operator=(kGraphicsPipeline&&) = delete;

		void createPipeline(GraphicsPipelineStruct pipelineStruct, const std::string& vertfilePath, const std::string& fragfilePath);
		void createRenderpass(VkFormat& swapchainImageFormat, VkFormat depthFormat);
		void bind(VkCommandBuffer commandBuffer);

		VkRenderPass getRenderPass() { return m_RenderPass; }
		VkPipeline getPipeline() { return m_GraphicsPipeline; }
		VkPipelineLayout getPipelineLayout() { return m_PipelineLayout; }
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
		/* depth stuff. */
		/*VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;*/

		/* vulkan device handle.*/
		Vulkan_Device& m_Device;
	};
}