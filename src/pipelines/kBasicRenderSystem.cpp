#include "kBasicRenderSystem.hpp"
#include "../kDevice.hpp"
#include "../kSwapChain.hpp"
#include "../kGraphicsPipeline.hpp"
#include "../kEntity.hpp"
#include "../kDescriptors.hpp"

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

	void kBasicRenderSystem::createGraphicsPipeline(VkDescriptorSetLayout layout)
	{
		GraphicsPipelineStruct pipelineStruct{};

		m_Basiclayout = LveDescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::array<VkDescriptorSetLayout, 2> layouts{
			layout,
			m_Basiclayout->getDescriptorSetLayout()
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();

		VkPushConstantRange cameraPushConstant{};
		cameraPushConstant.offset = 0;
		cameraPushConstant.size = sizeof(pushConstants);
		cameraPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
		pipelineLayoutInfo.pPushConstantRanges = &cameraPushConstant; // Optional

		VK_CHECK(vkCreatePipelineLayout(m_Device.m_Device, &pipelineLayoutInfo, nullptr, &m_BasicPipeLineLayout));

		pipelineStruct.viewportWidth = m_SwapChain.m_SwapChainExtent.width;
		pipelineStruct.viewportheight = m_SwapChain.m_SwapChainExtent.height;
		pipelineStruct.scissor.extent = m_SwapChain.m_SwapChainExtent;
		pipelineStruct.layout = m_BasicPipeLineLayout;
		

		m_GraphicsPipeline.createPipeline(pipelineStruct, "../shaders/vertexShader.spv", "../shaders/fragmentShader.spv");
	}
	void kBasicRenderSystem::renderEntities(glm::vec3 camerapos, glm::vec3 lightPos, glm::vec4 lightcolor, Frame& frameInfo)
	{
		m_GraphicsPipeline.bind(frameInfo.currentCommandBuffer);

		vkCmdBindDescriptorSets(frameInfo.currentCommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_BasicPipeLineLayout,
			0,
			1,
			&frameInfo.set,
			0,
			nullptr);

		for (auto& entity : frameInfo.entities)
		{
			auto& ent = entity.second;
			ent.getModel()->bind(frameInfo.currentCommandBuffer);

			std::vector<VkDescriptorImageInfo> imageInfos;
			for (size_t i = 0; i < ent.getModel()->m_Textures.size(); i++)
			{
				//VkDescriptorImageInfo info = entity.getModel()->m_Textures[i].getImageinfo();
				VkDescriptorImageInfo info{};
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				info.imageView = ent.getModel()->m_Textures[i].m_TextureVars.m_TextureView;
				info.sampler = ent.getModel()->m_Textures[i].m_TextureVars.m_Sampler;
				imageInfos.push_back(info);
			}

			auto bufferInfo = ent.getBufferInfo(frameInfo.CurrentFrameIndex);

			VkDescriptorSet set;
			LveDescriptorWriter(*m_Basiclayout, frameInfo.objPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfos[0])
				.writeImage(2, &imageInfos[1])
				.writeImage(3, &imageInfos[2])
				.writeImage(4, &imageInfos[3])
				.writeImage(5, &imageInfos[4])
				.build(ent.m_DescriptorSet);

			vkCmdBindDescriptorSets(frameInfo.currentCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_BasicPipeLineLayout,
				1,
				1,
				&ent.m_DescriptorSet,
				0,
				nullptr);

			pushConstants cameraConstants{};
			cameraConstants.cameraPosition = camerapos;
			cameraConstants.lightPosition = vars.m_LightPosition;
			cameraConstants.lighColor = lightcolor;
			cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, vars.m_Metalness, vars.m_Roughness);
			vkCmdPushConstants(frameInfo.currentCommandBuffer,
								m_BasicPipeLineLayout,
							   	VK_SHADER_STAGE_FRAGMENT_BIT,
							   	0,
							   sizeof(pushConstants),
							   	&cameraConstants);

			ent.getModel()->draw(frameInfo.currentCommandBuffer);
		}
	}
}