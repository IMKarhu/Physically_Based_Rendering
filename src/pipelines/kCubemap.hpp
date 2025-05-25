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

	class kCubeMap
	{
	public:
		kCubeMap(Vulkan_Device& device, Vulkan_SwapChain& swapchain);
		~kCubeMap();

		kCubeMap(const kCubeMap&) = delete;
		kCubeMap& operator = (const kCubeMap&) = delete;

		void createGraphicsPipeline(std::vector<VkDescriptorSetLayout> layouts);
		void renderCube(glm::vec3 cameraPos, Frame& frameInfo);
	private:

		Vulkan_Device& m_Device;
		Vulkan_SwapChain& m_SwapChain;
		std::unique_ptr<kGraphicsPipeline> m_CubeMapPipeline;
	};
}