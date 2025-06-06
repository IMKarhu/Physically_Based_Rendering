#pragma once
#include "utils/vkUtils.hpp"
#include "kDevice.hpp"
#include <vector>

namespace karhu
{
	struct Vulkan_SwapChain
	{
		Vulkan_SwapChain(Vulkan_Device& device);
		~Vulkan_SwapChain();

		Vulkan_SwapChain(const Vulkan_SwapChain&) = delete;
		void operator=(const Vulkan_SwapChain&) = delete;
		Vulkan_SwapChain(Vulkan_SwapChain&&) = delete;
		Vulkan_SwapChain& operator=(Vulkan_SwapChain&&) = delete;

		VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

		void createSwapChain(VkSurfaceKHR surface, GLFWwindow* window);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags); //refactor somewhere else, image class?
		void createImageViews();
		void createCommandBuffers();
		void createCommandPool();
		void createRenderPass(VkFormat depthFormat);
		VkCommandBuffer RecordSingleCommand();
		void endSingleCommand(VkCommandBuffer commandBuffer);

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		std::vector<VkImageView> m_SwapChainImageViews;
		const int m_MaxFramesInFlight = 2;
		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		VkRenderPass m_RenderPass;
	private:
		GLFWwindow* m_Window;
		Vulkan_Device& m_Device;
	};
}