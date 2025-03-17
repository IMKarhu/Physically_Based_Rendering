#pragma once
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "../frame.hpp"
#include "../types.hpp"

#include <vector>

namespace karhu
{
	struct Vulkan_Device;
	struct Vulkan_SwapChain;
	class kGraphicsPipeline;
	class kEntity;
	class LveDescriptorSetLayout;

	class kBasicRenderSystem
	{
	public:
		kBasicRenderSystem(Vulkan_Device& device, Vulkan_SwapChain& swapchain, kGraphicsPipeline& graphicspipeline);
		~kBasicRenderSystem();

		kBasicRenderSystem(const kBasicRenderSystem&) = delete;
		kBasicRenderSystem &operator = (const kBasicRenderSystem&) = delete;

		void createGraphicsPipeline(VkDescriptorSetLayout layout);
		
		void renderEntities(glm::vec3 camerapos, glm::vec3 lightPos, glm::vec4 lightcolor, Frame& frameInfo);
	private:

		Vulkan_Device& m_Device;
		Vulkan_SwapChain& m_SwapChain;
		kGraphicsPipeline& m_GraphicsPipeline;

		struct {
			float m_Metalness = 0.0f;
			float m_Roughness = 0.0f;
			glm::vec3 m_LightPosition = glm::vec3(1.0f, 3.0f, 1.0f);
		}vars;

		VkPipelineLayout m_BasicPipeLineLayout;
		std::unique_ptr<LveDescriptorSetLayout> m_Basiclayout;
	};
}