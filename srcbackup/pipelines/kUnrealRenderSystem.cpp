#include "kUnrealRenderSystem.hpp"
#include "../kDevice.hpp"
#include "../kSwapChain.hpp"
#include "../kGraphicsPipeline.hpp"

namespace karhu
{
	kUnrealRenderSystem::kUnrealRenderSystem(Vulkan_Device& device, Vulkan_SwapChain& swapchain)
		: m_Device(device)
		, m_SwapChain(swapchain)
	{
	}

	kUnrealRenderSystem::~kUnrealRenderSystem()
	{
	}

	void kUnrealRenderSystem::createGraphicsPipeline(std::vector<VkDescriptorSetLayout> layouts)
	{
		GraphicsPipelineStruct pipelineStruct{};
		pipelineStruct.viewportWidth = m_SwapChain.m_SwapChainExtent.width;
		pipelineStruct.viewportheight = m_SwapChain.m_SwapChainExtent.height;
		pipelineStruct.scissor.extent = m_SwapChain.m_SwapChainExtent;
		pipelineStruct.pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineStruct.pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineStruct.renderPass = m_SwapChain.m_RenderPass[Vulkan_SwapChain::renderPassType::NORMAL];

		m_UnrealEntityPipeline = std::make_unique<kGraphicsPipeline>(m_Device);
		m_UnrealEntityPipeline->createPipeline(pipelineStruct, "../shaders/unrealpbrvert.spv", "../shaders/unrealpbrfrag.spv");
	}
	void kUnrealRenderSystem::renderEntities(glm::vec3 cameraPos, glm::vec4 lightColor, Frame& frameInfo)
	{
		m_UnrealEntityPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_UnrealEntityPipeline->getPipelineLayout(),
			0,
			1,
			&frameInfo.globalSet,
			0,
			nullptr);

		for (auto& entity : frameInfo.unrealEntities)
		{

			vkCmdBindDescriptorSets(frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_UnrealEntityPipeline->getPipelineLayout(),
				1,
				1,
				&entity.m_DescriptorSet,
				0,
				nullptr);

			ObjPushConstant objConstant{};
			objConstant.model = entity.getTransformMatrix();
			vkCmdPushConstants(frameInfo.commandBuffer,
				m_UnrealEntityPipeline->getPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(ObjPushConstant),
				&objConstant);

			pushConstants cameraConstants{};
			cameraConstants.cameraPosition = cameraPos;
			cameraConstants.lightPosition = vars.m_LightPosition;
			cameraConstants.lighColor = lightColor;
			cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, vars.m_Metalness, vars.m_Roughness);
			vkCmdPushConstants(frameInfo.commandBuffer,
				m_UnrealEntityPipeline->getPipelineLayout(),
				VK_SHADER_STAGE_FRAGMENT_BIT,
				64,
				sizeof(pushConstants),
				&cameraConstants);


			entity.getModel()->bind(frameInfo.commandBuffer);
			entity.getModel()->draw(frameInfo.commandBuffer);
		}
	}
}