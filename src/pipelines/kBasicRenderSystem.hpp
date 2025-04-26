#pragma once
#include "../frame.hpp"
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include <vector>

namespace karhu
{
	class Vulkan_Device;
	class Vulkan_SwapChain;
	class kGraphicsPipeline;
	class kEntity;

	class kBasicRenderSystem
	{
	public:
		kBasicRenderSystem(Vulkan_Device& device, Vulkan_SwapChain& swapchain);
		~kBasicRenderSystem();

		kBasicRenderSystem(const kBasicRenderSystem&) = delete;
		kBasicRenderSystem &operator = (const kBasicRenderSystem&) = delete;

		void createGraphicsPipeline(std::vector<VkDescriptorSetLayout>);
		void renderEntities(glm::vec3 cameraPos, glm::vec4 lightColor, Frame& frameInfo);
	private:

		Vulkan_Device& m_Device;
		Vulkan_SwapChain& m_SwapChain;
		std::unique_ptr<kGraphicsPipeline> m_EntityPipeline;

		struct {
			float m_Metalness = 0.0f;
			float m_Roughness = 0.0f;
			glm::vec3 m_LightPosition = glm::vec3(1.0f, 3.0f, 1.0f);
		}vars;
	};
}