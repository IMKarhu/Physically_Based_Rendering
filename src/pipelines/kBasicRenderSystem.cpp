#include "kBasicRenderSystem.hpp"
#include "../kDevice.hpp"
#include "../kSwapChain.hpp"
#include "../kGraphicsPipeline.hpp"
#include "../kEntity.hpp"

namespace karhu
{
	kBasicRenderSystem::kBasicRenderSystem(Vulkan_Device& device, Vulkan_SwapChain& swapchain, kGraphicsPipeline& graphicspipeline)
		: m_Device(device)
		, m_SwapChain(swapchain)
		, m_GraphicsPipeline(graphicspipeline)
	{
	}

	kBasicRenderSystem::~kBasicRenderSystem()
	{
	}

	void kBasicRenderSystem::createGraphicsPipeline(VkDescriptorSetLayout SetLayout)
	{
		GraphicsPipelineStruct pipelineStruct{};
		pipelineStruct.viewportWidth = m_SwapChain.m_SwapChainExtent.width;
		pipelineStruct.viewportheight = m_SwapChain.m_SwapChainExtent.height;
		pipelineStruct.scissor.extent = m_SwapChain.m_SwapChainExtent;
		pipelineStruct.pipelineLayoutInfo.pSetLayouts = &SetLayout;

		m_GraphicsPipeline.createPipeline(pipelineStruct, "../shaders/vertexShader.spv", "../shaders/fragmentShader.spv");
	}
	void kBasicRenderSystem::renderEntities(std::vector<kEntity> entities, uint32_t currentFrameIndex, uint32_t index, glm::vec3 camerapos, glm::vec3 lightPos, glm::vec4 lightcolor, VkCommandBuffer commandBuffer)
	{
		m_GraphicsPipeline.bind(commandBuffer);

		for (auto& entity : entities)
		{
			entity.getModel()->bind(m_SwapChain.m_CommandBuffers[currentFrameIndex]);

			vkCmdBindDescriptorSets(m_SwapChain.m_CommandBuffers[currentFrameIndex],
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									m_GraphicsPipeline.getPipelineLayout(),
									0,
									1,
									&entity.m_DescriptorSet,
									0,
									nullptr);

			pushConstants cameraConstants{};
			cameraConstants.cameraPosition = camerapos;
			cameraConstants.lightPosition = vars.m_LightPosition;
			cameraConstants.lighColor = lightcolor;
			cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, vars.m_Metalness, vars.m_Roughness);
			vkCmdPushConstants(m_SwapChain.m_CommandBuffers[currentFrameIndex],
							   m_GraphicsPipeline.getPipelineLayout(),
							   	VK_SHADER_STAGE_FRAGMENT_BIT,
							   	0,
							   sizeof(pushConstants),
							   	&cameraConstants);

			entity.getModel()->draw(m_SwapChain.m_CommandBuffers[currentFrameIndex]);
		}
	}
}