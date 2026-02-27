#include "kCubeMap.hpp"
#include "../kDevice.hpp"
#include "../kSwapChain.hpp"
#include "../kGraphicsPipeline.hpp"

namespace karhu
{
	kCubeMap::kCubeMap(Vulkan_Device& device, Vulkan_SwapChain& swapchain)
		: m_Device(device)
		, m_SwapChain(swapchain)
	{
		m_Matrices = {
			glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),  // +X
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),  // -X
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),  // +Y
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),  // -Y
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),  // +Z
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))   // -Z
		};
		/*
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),*/
	}
	kCubeMap::~kCubeMap()
	{
	}
	void kCubeMap::createGraphicsPipeline(std::vector<VkDescriptorSetLayout>  layouts)
	{
		GraphicsPipelineStruct pipelineStruct{};
		pipelineStruct.viewportWidth = m_SwapChain.m_SwapChainExtent.width;
		pipelineStruct.viewportheight = m_SwapChain.m_SwapChainExtent.height;
		pipelineStruct.scissor.extent = m_SwapChain.m_SwapChainExtent;
		pipelineStruct.pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineStruct.pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineStruct.renderPass = m_SwapChain.m_RenderPass[Vulkan_SwapChain::renderPassType::CUBE];

		m_CubeMapPipeline = std::make_unique<kGraphicsPipeline>(m_Device);
		m_CubeMapPipeline->createPipeline(pipelineStruct, "../shaders/cubemapvert.spv", "../shaders/cubemapfrag.spv");
	}
	std::vector<VkFramebuffer> kCubeMap::createFrameBuffersCube(std::vector<VkFramebuffer> buffers, VkImage image)
	{
		m_FaceViews.resize(6);
		for (size_t i = 0; i < 6; i++)
		{
			VkImageViewCreateInfo faceViewInfo{};
			faceViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			faceViewInfo.image = image;
			faceViewInfo.format = VK_FORMAT_R32G32B32_SFLOAT;
			faceViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			faceViewInfo.subresourceRange.baseMipLevel = 0;
			faceViewInfo.subresourceRange.levelCount = 1;
			faceViewInfo.subresourceRange.baseArrayLayer = i;
			faceViewInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(m_Device.m_Device, &faceViewInfo, nullptr, &m_FaceViews[i]);
		}

		std::vector<VkFramebuffer> frameBuffers;
		for (uint32_t i = 0; i < 6; ++i) {
			VkFramebufferCreateInfo frameBufferInfo{};
			frameBufferInfo.renderPass = m_SwapChain.m_RenderPass[Vulkan_SwapChain::renderPassType::CUBE];
			frameBufferInfo.attachmentCount = 1;
			frameBufferInfo.pAttachments = &m_FaceViews[i];
			frameBufferInfo.width = 512;
			frameBufferInfo.height = 512;
			frameBufferInfo.layers = 1;

			vkCreateFramebuffer(m_Device.m_Device, &frameBufferInfo, nullptr, &frameBuffers[i]);
		}
		return frameBuffers;
	}
	void kCubeMap::renderCube(glm::vec3 cameraPos, Frame& frameInfo)
	{
		m_CubeMapPipeline->bind(frameInfo.commandBuffer);

		for (size_t i = 0; i < 6; i++)
		{
			updateCubeUbo(frameInfo, i);

			vkCmdBindDescriptorSets(frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_CubeMapPipeline->getPipelineLayout(),
				0,
				1,
				&frameInfo.globalCubeSet,
				0,
				nullptr);

			for (auto& entity : frameInfo.cubemap)
			{

				vkCmdBindDescriptorSets(frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_CubeMapPipeline->getPipelineLayout(),
					1,
					1,
					&entity.m_DescriptorSet,
					0,
					nullptr);

				ObjPushConstant objConstant{};
				objConstant.model = entity.getTransformMatrix();
				vkCmdPushConstants(frameInfo.commandBuffer,
					m_CubeMapPipeline->getPipelineLayout(),
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(ObjPushConstant),
					&objConstant);

				/*pushConstants cameraConstants{};
				cameraConstants.cameraPosition = cameraPos;
				cameraConstants.lightPosition = vars.m_LightPosition;
				cameraConstants.lighColor = lightColor;
				cameraConstants.albedoNormalMetalRoughness = glm::vec4(0.0f, 0.0f, vars.m_Metalness, vars.m_Roughness);
				vkCmdPushConstants(frameInfo.commandBuffer,
					m_CubeMapPipeline->getPipelineLayout(),
					VK_SHADER_STAGE_FRAGMENT_BIT,
					64,
					sizeof(pushConstants),
					&cameraConstants);*/


				
			}
		}
		frameInfo.cubemap[0].getModel()->bind(frameInfo.commandBuffer);
		frameInfo.cubemap[0].getModel()->draw(frameInfo.commandBuffer);
	}
	void kCubeMap::updateCubeUbo(Frame& frameInfo, uint32_t index)
	{
		for (auto& buffer : frameInfo.cubeBuffers)
		{
			UniformBufferObject obj;
			obj.view = m_Matrices[index];
			obj.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			obj.proj[1][1] *= -1;

			memcpy(buffer->m_BufferMapped, &obj, sizeof(obj));
		}
	}
}