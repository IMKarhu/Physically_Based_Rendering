#pragma once
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

namespace karhu
{
	class Vulkan_Device;
	class Vulkan_SwapChain;
	class kGraphicsPipeline;
	class kEntity;

	class kBasicRenderSystem
	{
	public:
		kBasicRenderSystem(Vulkan_Device& device, Vulkan_SwapChain& swapchain, kGraphicsPipeline& graphicspipeline);
		~kBasicRenderSystem();

		kBasicRenderSystem(const kBasicRenderSystem&) = delete;
		kBasicRenderSystem &operator = (const kBasicRenderSystem&) = delete;

		void createGraphicsPipeline(VkDescriptorSetLayout SetLayout);
		/*void recordCommandBuffer(kEntity& entity, uint32_t currentFrameIndex, uint32_t index, glm::vec3 position, glm::vec3 lightPos, glm::vec4 lightColor);*/
		/*frameindex, dt, commandbuffer, camera, descriptor set, entities*/
		void renderEntities(std::vector<kEntity> entities, uint32_t currentFrameIndex, uint32_t index, glm::vec3 camerapos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 lightPos, glm::vec4 lightcolor, VkCommandBuffer commandBuffer);
	private:

		Vulkan_Device& m_Device;
		Vulkan_SwapChain& m_SwapChain;
		kGraphicsPipeline& m_GraphicsPipeline;

		struct {
			float m_Metalness = 0.0f;
			float m_Roughness = 0.0f;
			glm::vec3 m_LightPosition = glm::vec3(1.0f, 3.0f, 1.0f);
		}vars;
	};
}