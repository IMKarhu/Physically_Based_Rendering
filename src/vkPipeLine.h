#pragma once
#include "vulkan/vulkan.h"
#include <string>
#include <vector>

namespace kge
{
	class vkDevice;
	class kgeModel;

	struct vkPipelineConfigInfo
	{
		vkPipelineConfigInfo() = default;
		vkPipelineConfigInfo(const vkPipelineConfigInfo&) = delete;
		vkPipelineConfigInfo& operator=(const vkPipelineConfigInfo&) = delete;

		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		VkPipelineMultisampleStateCreateInfo multiSampling{};
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		VkPipelineLayout pipeLineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};
	class vkPipeline
	{
	public:
		vkPipeline(vkDevice* device, const std::string& vertfilepath, const std::string& fragfilepath, const vkPipelineConfigInfo& configInfo);
		vkPipeline(const vkPipeline&) = delete; /* Disallows copying. */
		~vkPipeline();

		vkPipeline& operator=(const vkPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		static vkPipelineConfigInfo& createPipeLineConfigInfo(vkPipelineConfigInfo& configInfo, uint32_t width, uint32_t height);

	private:
		VkPipeline m_Pipeline;
		VkShaderModule m_VertShaderModule;
		VkShaderModule m_FragShaderModule;
		vkDevice* m_Device;

		void createPipeLine(const std::string& vertfilepath, const std::string& fragfilepath, const vkPipelineConfigInfo& configInfo);
		VkShaderModule createShaderModule(const std::vector<char>& code);
		static std::vector<char> readFile(const std::string& filename);
	};
}