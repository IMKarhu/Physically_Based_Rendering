#pragma once
#include "vulkan/vulkan.h"
#include <iostream>
#include <array>
#include <vector>

namespace kge
{
	class Command
	{
	public:
		Command();
		void createCommandPool(const VkDevice& device,  const VkPhysicalDevice& pdevice, const VkSurfaceKHR& surface);
		void createCommandBuffer(const VkDevice& device, const int maxframesinflight);
		void recordCommandBuffer(const VkExtent2D& swapChainExtent, const std::vector<VkFramebuffer>& frameBuffers,
								 const VkRenderPass& renderPass, VkCommandBuffer commandBuffer, uint32_t imageIndex,
								 const VkPipeline& pipeline, const VkBuffer& vertexbuffer, const VkBuffer& indexbuffer,
								 const std::vector<VkDescriptorSet>& DSset, int currentFrame, uint32_t indexCount, const VkPipelineLayout& layout);
		VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandpool, const VkDevice& device);
		void endSingleTimeCommands(VkCommandPool commandpool, VkCommandBuffer commandBuffer, const VkDevice& device, const VkQueue& graphicsqueue);

		[[nodiscard]] const VkCommandPool getCommandPool() const { return m_CommandPool; }
		[[nodiscard]] std::vector<VkCommandBuffer> getCommandBuffers()  { return m_CommandBuffers; }

	private:
		/* Class members. */
		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}