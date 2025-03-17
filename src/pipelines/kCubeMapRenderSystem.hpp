#pragma once
#include "vulkan/vulkan.h"


namespace karhu
{
	struct Vulkan_Device;
	struct Vulkan_SwapChain;
	class kGraphicsPipeline;
	class kEntity;

	class kCubeMapRenderSystem
	{
	public:
		kCubeMapRenderSystem(Vulkan_Device& device, Vulkan_SwapChain& swapchain, kGraphicsPipeline& graphicspipeline);
		~kCubeMapRenderSystem();

		kCubeMapRenderSystem(const kCubeMapRenderSystem&) = delete;
		kCubeMapRenderSystem& operator = (const kCubeMapRenderSystem&) = delete;

		void createGraphicsPipeline(VkDescriptorSetLayout SetLayout);

	private:
		Vulkan_Device& m_Device;
		Vulkan_SwapChain& m_SwapChain;
		kGraphicsPipeline& m_GraphicsPipeline;
	};
}